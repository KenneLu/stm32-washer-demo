#include "OLED_Data.h"

/*图像数据*********************/
const uint8_t Cursor[] = { 0x10,0x38,0x54,0x92,0x10,0x10,0x10,0x10, };

const uint8_t goutou[] = {
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0xF7,0xAB,0xFF,0xAF,0xDF,0xFF,0x3F,0xFF,0xBF,
0xDF,0xFF,0x5F,0xFF,0xBF,0xDF,0x6F,0xF7,0x9B,0xEF,0xB5,0x6B,0xBF,0xFF,0xFF,0xFF,
0xFF,0xFF,0x1F,0xE7,0xF9,0xCF,0xFF,0x0E,0x1F,0x0B,0x1E,0xBF,0xF5,0xAF,0x5F,0xDB,
0xEE,0xDF,0xEF,0xDD,0xEF,0x1B,0x0F,0x1E,0x1B,0xFE,0xF7,0x59,0xFF,0xAF,0xFF,0xFF,
0xFF,0xFF,0x57,0xFC,0xFF,0xFE,0xFD,0xBF,0x7A,0x92,0x82,0x33,0x79,0x7F,0x7F,0x3C,
0xBF,0xFE,0xFD,0xFA,0xFF,0xF6,0xFA,0xFE,0xF7,0xFD,0xFF,0x7D,0xBF,0xEA,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFA,0xFF,0xEB,0xFF,0xF7,0xDF,0xEF,0xFF,0xDF,0xFF,0xDE,0xFF,0xDF,
0xFF,0xDF,0xBF,0xEF,0xDF,0xFF,0xE7,0xFF,0xF7,0xFB,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,
};

//壁纸
const uint8_t Wallpaper[] = {
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xD7,0xAF,0xFF,0xED,0xBF,0xFF,0x7F,0xF5,0x9A,0x65,0x1A,0xF5,0x0B,0xF7,0x2C,0xD0 ,
 0xB0,0x50,0xA4,0xC0,0x60,0xF4,0xA0,0xF0,0xF0,0xB0,0xE8,0xF0,0x70,0xD0,0xB8,0xF0 ,
 0xB8,0xF0,0x58,0xF8,0xD8,0x7C,0xB0,0xE8,0xBC,0xD6,0x65,0xBD,0x42,0xBD,0x42,0x2D ,
 0x80,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x20,0x00,0x00 ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0x01,0x00,0x01,0x00,0x01,0x00,0x09,0x00,0x06,0x03,0x06,0x39,0xEE,0xB9,0xD7,0x6C ,
 0xFB,0xEF,0xFA,0xBF,0x75,0x3F,0xAF,0x1A,0x1F,0x1E,0xAB,0x3F,0xF7,0xED,0xBF,0xFA ,
 0xEF,0x7B,0xDF,0x6A,0xFF,0x55,0x1B,0x4E,0x05,0x0A,0x27,0x8D,0xE6,0xBB,0xED,0x54 ,
 0xB0,0xC0,0x00,0x00,0x04,0x00,0x40,0x00,0x01,0x00,0x00,0x08,0x00,0x00,0x00,0x00 ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFA,0xFF,0xEF,0xFD,0xBF,0xF7 ,
 0xFF,0xDD,0xFE,0xEF,0xFD,0xBD,0xFC,0xF4,0x78,0xFA,0xFC,0xFA,0xFE,0xFB,0xFF,0xFF ,
 0x5F,0x2B,0xBF,0x45,0x3F,0x8B,0xAE,0xFB,0xDE,0xEE,0xBB,0xFE,0x57,0xFA,0x5F,0xF5 ,
 0xAA,0x7F,0x54,0x44,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x01,0x00,0x00,0x00,0x00 ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x2F,0x7F,0xFD,0xFF,0xEF ,
 0xFD,0x7F,0xF7,0xDE,0x7F,0xFB,0x5F,0xFF,0xDB,0xBF,0xFF,0xB6,0xFF,0xAF,0x7B,0x9F ,
 0xD7,0x5F,0xE6,0x99,0x62,0xDF,0x65,0x9F,0x6A,0x9F,0x62,0xBF,0xD5,0xBB,0xCF,0x72 ,
 0x5F,0x15,0x41,0x04,0x80,0x40,0x00,0x01,0x80,0x08,0x00,0x00,0x22,0x00,0x08,0x00 ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0x80,0xC0,0x40,0x60,0x10,0xA0,0x58,0x00,0x14,0x40,0x0A,0x90,0x42,0x09,0xAF,0xEE ,
 0xFF,0xFF,0xED,0xFF,0xBD,0xF7,0xFD,0xAF,0xFA,0x57,0xFA,0x5F,0xE4,0xBB,0xCD,0x77 ,
 0x95,0xEB,0x34,0xCB,0xF6,0x89,0xF6,0x59,0xE6,0x5B,0xF5,0xAE,0xF3,0x4E,0xF9,0x56 ,
 0xF1,0x44,0xD1,0x04,0x40,0x14,0x01,0x44,0x10,0x82,0x08,0x41,0x10,0x04,0x41,0x08 ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFD,0xF8,0x7A,0xF5,0xF8,0xF2,0xB8,0xF2,0x71,0xE0,0xE8,0x72,0xE0,0xA8,0xFA,0xDF ,
 0x7F,0xEE,0xBF,0xFF,0xAF,0xFD,0xDF,0xF6,0x5F,0xBF,0xB5,0xFF,0x2F,0xFC,0x57,0xFB ,
 0x5E,0xFB,0xAF,0xFA,0x5F,0xEA,0x7F,0xF5,0xAF,0xFD,0x77,0xBA,0xEF,0x7D,0xBF,0xEB ,
 0x7E,0xAB,0x7D,0xD7,0x6C,0x90,0x01,0x00,0x84,0x00,0x10,0x82,0x00,0x51,0x04,0x00 ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0xBF,0x07,0xBF,0xFF ,
 0x9F,0xDF,0x57,0xCF,0xDF,0x9F,0xFF,0xEF,0x6F,0x8F,0xEF,0xFF,0x5F,0x67,0x6F,0x2F ,
 0x4F,0x7F,0xFF,0x7F,0x75,0x6E,0xFF,0xFF,0xFF,0xFE,0x07,0xFD,0xFF,0xFD,0xFE,0x5F ,
 0x65,0x6F,0xDD,0x1F,0xBD,0xAF,0x06,0xAF,0x0D,0xBF,0xFE,0x7F,0x77,0x6F,0xFD,0x0E ,
 0xAF,0xAD,0xAE,0xB7,0xB5,0x37,0xFD,0xFF,0x0E,0xEF,0xEF,0x6E,0x6D,0x6F,0x6E,0xED ,
 0xEE,0x0F,0xFC,0xFF,0xBC,0xBF,0xBD,0xB6,0x2D,0xBE,0xBD,0xBE,0xBD,0xFE,0xFF,0x6C ,
 0x6F,0x6F,0x47,0x6F,0x6F,0x6F,0x87,0xAF,0xAF,0xEF,0xFF,0xFF,0x1F,0x57,0x4F,0x5F ,
 0x1F,0x5F,0x4F,0x57,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF ,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xBB,0x80,0xFD,0xBF ,
 0xBD,0xD1,0xEC,0xED,0xD1,0xBD,0xFF,0xFD,0xC0,0xEE,0xC0,0xBF,0xC0,0xF5,0xC0,0xF5 ,
 0xB5,0x80,0xFF,0xFF,0xFF,0xC0,0xEF,0xB7,0xCF,0xF1,0xFE,0xF1,0xCF,0xBF,0xFF,0xFB ,
 0x80,0xDB,0xAC,0xC1,0xAB,0xAA,0x80,0xAA,0xAA,0xAF,0xFF,0xBF,0xDF,0xE0,0xCF,0xB0 ,
 0xBF,0xAE,0xB5,0xBB,0xB5,0xAE,0xFF,0xFF,0x80,0xDF,0xDF,0xD8,0xDB,0xDB,0xD8,0xDF ,
 0xDF,0x80,0xFF,0xBF,0xBB,0xBB,0xBB,0xBB,0x80,0xBB,0xBB,0xBB,0xBB,0xBF,0xFF,0xB7 ,
 0xB6,0xD5,0xE7,0xF6,0x81,0xF7,0xE5,0xD6,0xB7,0xB7,0xFF,0xF7,0xF4,0xF5,0xF5,0xF5 ,
 0x80,0xF5,0xF5,0xF5,0xF4,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/*********************图像数据*/

