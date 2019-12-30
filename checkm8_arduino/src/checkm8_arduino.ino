#include "../include/User_Setup.h"
#include "../include/Usb.h"
#include "checkm8_config.h"
#include "ard_protocol.h"

USB Usb;
USB_DEVICE_DESCRIPTOR desc_buf;
uint8_t state, rcode, addr = 1;
uint8_t usb_data_buf[ARD_BUF_SIZE];

struct serial_desc_args sd_args;
struct usb_xfer_args usb_args;

int i, chunk_i;
int size, chunk_size;
char cmd;

void recv_args(uint8_t *target, int len)
{
    for(i = 0; i < len; i = i + 1)
    {
        while(Serial.available() == 0);

        if(target == NULL) Serial.read();
        else target[i] = (uint8_t) Serial.read();
    }
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

uint8_t send_data(uint8_t *buf, uint8_t len)
{
    Usb.bytesWr(rSNDFIFO, len, buf);
    Usb.regWr(rSNDBC, len);
    Usb.regWr(rHXFR, tokOUT);
    while(!(Usb.regRd(rHIRQ) & bmHXFRDNIRQ));
    Usb.regWr(rHIRQ, bmHXFRDNIRQ);
    return (Usb.regRd(rHRSL) & 0x0f);
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
    state = Usb.getUsbTaskState();
    while(state != USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE && state != USB_STATE_RUNNING)
    {
        Usb.Task();
        state = Usb.getUsbTaskState();
    }

    if(Serial.available() > 0)
    {
        cmd = (char) Serial.read();
        switch(cmd)
        {
            case PROT_PARTIAL_CTRL_XFER:
                recv_args((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);
                if(respond_rcode()) break;

                if(usb_args.bmRequestType & 0x80u)
                {
                    Usb.regWr(rHCTL, bmRCVTOG1);
                    rcode = Usb.dispatchPkt(tokIN, 0, 0);
                }
                else rcode = Usb.dispatchPkt(tokOUTHS, 0, 0);

                if(respond_rcode()) break;
                Serial.write(PROT_SUCCESS);
                break;

            case PROT_NO_ERROR_CTRL_XFER:
                recv_args((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);
                respond_rcode();

                if(usb_args.bmRequestType & 0x80u)
                {
                    Usb.regWr(rHCTL, bmRCVTOG1);
                    rcode = Usb.dispatchPkt(tokIN, 0, 0);
                }
                else rcode = Usb.dispatchPkt(tokOUTHS, 0, 0);

                respond_rcode();
                Serial.write(PROT_SUCCESS);
                break;

            case PROT_NO_ERROR_CTRL_XFER_DATA:
                recv_args((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);
                respond_rcode();
                Usb.regWr(rHCTL, bmSNDTOG0);

                rcode = send_data(usb_data_buf, 0);
                respond_rcode();

                chunk_i = 0;
                while(chunk_i < usb_args.data_len)
                {
                    if(usb_args.data_len - chunk_i > ARD_BUF_SIZE) chunk_size = ARD_BUF_SIZE;
                    else chunk_size = usb_args.data_len - chunk_i;

                    recv_args(usb_data_buf, chunk_size);
                    Serial.write(PROT_ACK);

                    // i is the current data index
                    i = 0;
                    while(i < chunk_size)
                    {
                        if(chunk_size - i > 64) size = 64;
                        else size = chunk_size - i;

                        rcode = send_data(&usb_data_buf[i], size);
                        respond_rcode();
                        i += size;
                    }

                    chunk_i += chunk_size;
                }

                Serial.write(PROT_SUCCESS);
                break;

//            case PROT_CTRL_XFER:
//                recv_args((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
//                if(receive_data_and_respond()) break;
//
//                rcode = Usb.ctrlReq(addr, 0,
//                                    usb_args.bmRequestType,
//                                    usb_args.bRequest,
//                                    usb_args.wValue & 0xFFu,
//                                    (usb_args.wValue >> 8u) & 0xFFu,
//                                    usb_args.wIndex,
//                                    usb_args.data_len, usb_args.data_len,
//                                    usb_data_buf, NULL);
//                if(respond_rcode()) break;
//
//                Serial.write(PROT_SUCCESS);
//                break;

            case PROT_RESET:
                Serial.write(PROT_ACK);

                Usb.setUsbTaskState(USB_ATTACHED_SUBSTATE_RESET_DEVICE);
                while((state = Usb.getUsbTaskState()) != USB_STATE_RUNNING) Usb.Task();

                Serial.write(PROT_SUCCESS);
                break;

            case PROT_SERIAL_DESC:
                recv_args((uint8_t *) &sd_args, sizeof(struct serial_desc_args));
                Serial.write(PROT_ACK);

                state = Usb.getUsbTaskState();
                if(state == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE)
                {
                    Serial.write(PROT_FAIL_NODEV);
                    break;
                }

                Usb.getDevDescr(addr, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t *) &desc_buf);
                if(desc_buf.idVendor != sd_args.dev_idVendor ||
                   desc_buf.idProduct != sd_args.dev_idProduct)
                {
                    Serial.write(PROT_FAIL_WRONGDEV);
                    break;
                }

                // multiplication by 2 is necessary here because iphone returns 16-bit characters
                Usb.getStrDescr(addr, 0, sd_args.len * 2, desc_buf.iSerialNumber, 0x0409, usb_data_buf);
                Serial.write(PROT_SUCCESS);

                // not sure what the first byte is; skip it
                for(i = 1; i < sd_args.len + 1; i++)
                {
                    Serial.write(((uint16_t *) usb_data_buf)[i]);
                }
                break;

//            default:
//                Serial.write(PROT_FAIL_BADCMD);
//                break;
        }
    }
}