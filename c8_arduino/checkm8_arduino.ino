#include "include/User_Setup.h"
#include "include/Usb.h"
#include "checkm8_config.h"
#include "ard_protocol.h"

USB Usb;
uint8_t state, rcode, addr = 1;
uint8_t usb_data_buf[ARD_BUF_SIZE];

uint8_t desc_buf_val = 0;
USB_DEVICE_DESCRIPTOR desc_buf;
struct serial_desc_args sd_args;
struct usb_xfer_args usb_args;

unsigned int i, chunk_i;
unsigned int size, chunk_size;
int nak_count;
char cmd;

void recv_serial(uint8_t *target, unsigned int len)
{
    for(i = 0; i < len; i = i + 1)
    {
        while(Serial.available() == 0);

        if(target == NULL) Serial.read();
        else target[i] = (uint8_t) Serial.read();
    }
}

void get_dev_descriptor()
{
    if(!desc_buf_val)
    {
        Usb.getDevDescr(addr, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t *) &desc_buf);
        desc_buf_val = 1;
    }
}


uint8_t send_usb(uint8_t *buf, uint8_t len)
{
    Usb.bytesWr(rSNDFIFO, len, buf);
    Usb.regWr(rSNDBC, len);
    Usb.regWr(rHXFR, tokOUT);
    while(!(Usb.regRd(rHIRQ) & bmHXFRDNIRQ));
    Usb.regWr(rHIRQ, bmHXFRDNIRQ);
    return (Usb.regRd(rHRSL) & 0x0f);
}

uint8_t respond_rcode()
{
    if(rcode)
    {
        Serial.write(PROT_FAIL_USB);
        Serial.write(rcode);
        return 1;
    }
    else return 0;
}


void setup()
{
    Serial.begin(ARDUINO_BAUD);
    while(Serial.available() > 0) Serial.read();

    if(Usb.Init() == -1) Serial.write(PROT_FAIL_INITUSB);
    else Serial.write(PROT_SUCCESS);
}

void loop()
{
    Usb.Task();
    state = Usb.getUsbTaskState();
    while(state != USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE && state != USB_STATE_RUNNING)
    {
        Usb.Task();
        state = Usb.getUsbTaskState();
    }

    if(state = USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE)
    {
        desc_buf_val = 0;
    }

    if(Serial.available() > 0)
    {
        cmd = (char) Serial.read();
        switch(cmd)
        {
            case PROT_PARTIAL_CTRL_XFER:
                recv_serial((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);
                if(respond_rcode()) return;

                if(usb_args.bmRequestType & 0x80u)
                {
                    Usb.regWr(rHCTL, bmRCVTOG1);
                    rcode = Usb.dispatchPkt(tokIN, 0, 0);
                }
                else rcode = Usb.dispatchPkt(tokOUTHS, 0, 0);

                if(respond_rcode()) return;
                Serial.write(PROT_SUCCESS);
                break;

            case PROT_NO_ERROR_CTRL_XFER:
                recv_serial((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);

                if(usb_args.bmRequestType & 0x80u)
                {
                    Usb.regWr(rHCTL, bmRCVTOG1);
                    rcode = Usb.dispatchPkt(tokIN, 0, 0);
                }
                else rcode = Usb.dispatchPkt(tokOUTHS, 0, 0);

                Serial.write(PROT_SUCCESS);
                return;

            case PROT_NO_ERROR_CTRL_XFER_DATA:
                recv_serial((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);
                Usb.regWr(rHCTL, bmSNDTOG0);
                rcode = send_usb(usb_data_buf, 0);

                chunk_i = 0;
                while(chunk_i < usb_args.data_len)
                {
                    if(usb_args.data_len - chunk_i > ARD_BUF_SIZE) chunk_size = ARD_BUF_SIZE;
                    else chunk_size = usb_args.data_len - chunk_i;

                    Serial.write((unsigned char *) &chunk_size, 2);
                    recv_serial(usb_data_buf, chunk_size);

                    i = 0; // current data index
                    while(i < chunk_size)
                    {
                        if(chunk_size - i > 64) size = 64;
                        else size = chunk_size - i;

                        rcode = send_usb(&usb_data_buf[i], size);
                        i += size;
                    }

                    chunk_i += chunk_size;
                }

                Serial.write(PROT_SUCCESS);
                return;

            case PROT_CTRL_XFER:
                recv_serial((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                Serial.write(PROT_ACK);

                get_dev_descriptor();

                if(usb_args.trigger == 1) digitalWrite(6, HIGH);
                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);
                if(usb_args.trigger == 1) digitalWrite(6, LOW);

                if(usb_args.bmRequestType & 0x80u)
                {
                    i = 0;
                    Usb.regWr(rHCTL, bmRCVTOG1);

                    while(i < usb_args.data_len)
                    {
                        Usb.regWr(rHXFR, tokIN);

                        while(!(Usb.regRd(rHIRQ) & bmHXFRDNIRQ));
                        Usb.regWr(rHIRQ, bmHXFRDNIRQ);

                        if(Usb.regRd(rHIRQ) & bmRCVDAVIRQ)
                        {
                            if(usb_args.trigger == 1)
                            {
                                digitalWrite(6, HIGH);
                                delayMicroseconds(15);
                                digitalWrite(6, LOW);
                            }

                            size = Usb.regRd(rRCVBC);
                            Usb.bytesRd(rRCVFIFO, size, usb_data_buf);
                            Usb.regWr(rHIRQ, bmRCVDAVIRQ);

                            Serial.write((unsigned char *) &size, 2);
                            Serial.write(usb_data_buf, size);
                            i += size;

                            if(size != desc_buf.bMaxPacketSize0) break;
                        }
                        else
                        {
                            rcode = (Usb.regRd(rHRSL) & 0x0f);
                            if(rcode == hrNAK) continue;

                            Serial.write(PROT_FAIL_USB);
                            Serial.write(rcode);
                            return;
                        }
                    }

                    Usb.regWr(rHXFR, tokOUTHS);
                    Serial.write(PROT_SUCCESS);
                    return;
                }
                else
                {
                    chunk_i = 0;
                    Usb.regWr(rHCTL, bmSNDTOG0);

                    rcode = send_usb(usb_data_buf, 0);
                    while(chunk_i < usb_args.data_len)
                    {
                        if(usb_args.data_len - chunk_i > ARD_BUF_SIZE) chunk_size = ARD_BUF_SIZE;
                        else chunk_size = usb_args.data_len - chunk_i;

                        Serial.write((unsigned char *) &chunk_size, 2);
                        recv_serial(usb_data_buf, chunk_size);

                        i = 0;
                        while(i < chunk_size)
                        {
                            if(chunk_size - i > desc_buf.bMaxPacketSize0) size = desc_buf.bMaxPacketSize0;
                            else size = chunk_size - i;

                            rcode = send_usb(&usb_data_buf[i], size);
                            i += size;
                        }

                        chunk_i += chunk_size;
                    }

                    Usb.regWr(rHXFR, tokINHS);
                    Serial.write(PROT_SUCCESS);
                    return;
                }

            case PROT_RESET:
                Serial.write(PROT_ACK);

                Usb.setUsbTaskState(USB_ATTACHED_SUBSTATE_RESET_DEVICE);
                while((state = Usb.getUsbTaskState()) != USB_STATE_RUNNING) Usb.Task();

                Serial.write(PROT_SUCCESS);
                return;

            case PROT_SERIAL_DESC:
                recv_serial((uint8_t *) &sd_args, sizeof(struct serial_desc_args));
                Serial.write(PROT_ACK);

                state = Usb.getUsbTaskState();
                if(state == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE)
                {
                    Serial.write(PROT_FAIL_NODEV);
                    return;
                }

                get_dev_descriptor();
                if(desc_buf.idVendor != sd_args.dev_idVendor ||
                   desc_buf.idProduct != sd_args.dev_idProduct)
                {
                    Serial.write(PROT_FAIL_WRONGDEV);
                    return;
                }

                // multiplication by 2 is necessary here because iphone returns 16-bit characters
                Usb.getStrDescr(addr, 0, sd_args.len * 2, desc_buf.iSerialNumber, 0x0409, usb_data_buf);
                Serial.write(PROT_SUCCESS);

                // not sure what the first byte is; skip it
                for(i = 1; i < sd_args.len + 1; i++)
                {
                    Serial.write(((uint16_t *) usb_data_buf)[i]);
                }
                return;

            default:
                Serial.write(PROT_FAIL_BADCMD);
                return;
        }
    }
}