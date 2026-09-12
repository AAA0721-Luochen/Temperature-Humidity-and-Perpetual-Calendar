


u16 LCD_BGR2RGB(u16 c);
void Gui_Circle(u16 X,u16 Y,u16 R,u16 fc); 
void Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color);  
void Gui_box(u16 x, u16 y, u16 w, u16 h,u16 bc);
void Gui_box2(u16 x,u16 y,u16 w,u16 h, u8 mode);
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2);
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2);
void Gui_DrawFont_GBK16(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void Gui_DrawFont_GBK24(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void Gui_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num) ;
void Gui_showimage(const unsigned char *p, uint8_t c, uint8_t k, uint8_t x, uint8_t y);
void Font_Test(void);

/* 显示 Font.h 中的 32×32 用户图片，x、y 为显示位置参数。 */
void Gui_ShowUserImage(uint8_t x, uint8_t y);

/* 显示 Font.h 中的 32×32 湿度图标。 */
void Gui_ShowHumidityIcon(uint8_t x, uint8_t y);
