// WIP

/**
 * FAIRY COMPUTER SYSTEM 80 - Video Display Processor Emulator
 * -----------------------------------------------------------------------------
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Yoji Suzuki.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#ifndef INCLUDE_FCS80VIDEO_HPP
#define INCLUDE_FCS80VIDEO_HPP
#include <stdio.h>
#include <string.h>

class FCS80Video {
    private:
        void (*detectEndOfFrame)(void* arg);
        void (*detectIRQ)(void* arg);
        void* arg;
        inline unsigned char* getBgNameTableAddr() { return &this->ctx.ram[0x0000]; }
        inline unsigned char* getBgAttrTableAddr() { return &this->ctx.ram[0x0400]; }
        inline unsigned char* getFgNameTableAddr() { return &this->ctx.ram[0x0800]; }
        inline unsigned char* getFgAttrTableAddr() { return &this->ctx.ram[0x0C00]; }
        inline unsigned short* getColorTableAddr() { return (unsigned short*)&this->ctx.ram[0x1400]; }
        inline unsigned char* getPatternTableAddr() { return &this->ctx.ram[0x2000]; }
        inline unsigned char getRegisterBgScrollX() { return this->ctx.ram[0x1602]; }
        inline unsigned char getRegisterBgScrollY() { return this->ctx.ram[0x1603]; }
        inline unsigned char getRegisterFgScrollX() { return this->ctx.ram[0x1604]; }
        inline unsigned char getRegisterFgScrollY() { return this->ctx.ram[0x1605]; }
        inline unsigned char getRegisterIRQ() { return this->ctx.ram[0x1606]; }

    public:
        unsigned short display[240 * 192];
        struct Context {
            int bobo;
            int countV;
            unsigned char ram[0x4000];
            unsigned char countH;
        } ctx;

        FCS80Video(void* arg, void (*detectEndOfFrame)(void* arg), void (*detectIRQ)(void* arg)) {
            this->detectEndOfFrame = detectEndOfFrame;
            this->detectIRQ = detectIRQ;
            this->arg = arg;
        }

        void reset() { memset(&this->ctx, 0, sizeof(this->ctx)); }

        inline unsigned char read(unsigned short addr) {
            addr &= 0x3FFF;
            switch (addr) {
                case 0x1600: return this->ctx.countV < 200 ? this->ctx.countV : 0xFF;
                case 0x1601: return this->ctx.countH;
                default: return this->ctx.ram[addr];
            }
        }

        inline void write(unsigned short addr, unsigned char value) { this->ctx.ram[addr & 0x3FFF] = value; }

        inline void tick() {
            this->ctx.countH++;
            if (0 == this->ctx.countH) {
                this->renderScanline(this->ctx.countV);
                this->ctx.countV++;
                this->ctx.countV %= 262;
                if (0 == this->ctx.countV) {
                    this->detectEndOfFrame(this->arg);
                } else if (this->ctx.countV == this->getRegisterIRQ()) {
                    this->detectIRQ(this->arg);
                }
            }
        }

    private:
        inline void renderScanline(int scanline) {
            if (scanline < 8 || 200 <= scanline) return; // blanking
            scanline -= 8;
            this->renderBG(scanline);
            this->renderSprites(scanline);
            this->renderFG(scanline);
        }

        inline void renderBG(int scanline) {
            int y = scanline + this->getRegisterBgScrollY();
            unsigned short* display = &this->display[scanline * 240];
            unsigned short* colorTable = getColorTableAddr();
            int offset = (y / 8) * 64;
            unsigned char* nametbl = this->getBgNameTableAddr() + offset;
            unsigned char* attrtbl = this->getBgAttrTableAddr() + offset;
            unsigned short* colortbl = this->getColorTableAddr();
            for (int x = this->getRegisterBgScrollX() + 8, xx = 0; xx < 240; x++, xx++, display++) {
                offset = (x >> 3) & 0x1F;
                unsigned char ptn = nametbl[offset];
                unsigned char attr = attrtbl[offset];
                unsigned char* chrtbl = this->getPatternTableAddr() + (ptn << 5) + ((y >> 3) << 2) + ((x & 7) >> 1);
                int pal = x & 1 ? (*chrtbl) & 0x0F : ((*chrtbl) & 0xF0) >> 4;
                pal += (attr & 0x0F) << 4;
                *display = colortbl[pal];
            }
        }

        inline void renderFG(int scanline) {
        }

        inline void renderSprites(int scanline) {
        }
};

#endif
