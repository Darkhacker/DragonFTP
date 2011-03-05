void drawText(u32 font_offset, int size, char input[], int posx, int posy) {
 float x,y;
 x = posx;
 y = posy; 
 for(int i = 0; i < strlen(input); i++) {

   switch(input[i]) {
   case '/':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 15,2, 16, 16);
   break;
   case '.':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 14,2, 16, 16);
   break;
   case '0':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 0, 3, 16, 16);
   break;
   case '1':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 1, 3, 16, 16);
   break;
   case '2':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 2, 3, 16, 16);
   break;
   case '3':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 3, 3, 16, 16);
   break;
   case '4':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 4, 3, 16, 16);
   break;
   case '5':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 5, 3, 16, 16);
   break;
   case '6':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 6, 3, 16, 16);
   break;
   case '7':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 7, 3, 16, 16);
   break;
   case '8':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 8, 3, 16, 16);
   break;
   case '9':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 9, 3, 16, 16);
   break;
   case 'A':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 1, 4, 16, 16);
   break;
   case 'B':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 2, 4, 16, 16);
   break;
   case 'C':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 3, 4, 16, 16);
   break;
   case 'D':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 4, 4, 16, 16);
   break;
   case 'E':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 5, 4, 16, 16);
   break;
   case 'F':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 6, 4, 16, 16);
   break;
   case 'G':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 7, 4, 16, 16);
   break;
   case 'H':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 8, 4, 16, 16);
   break;
   case 'I':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 9, 4, 16, 16);
   break;
   case 'J':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 10, 4, 16, 16);
   break;
   case 'K':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 11, 4, 16, 16);
   break;
   case 'L':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 12, 4, 16, 16);
   break;
   case 'M':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 13, 4, 16, 16);
   break;
   case 'N':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 14, 4, 16, 16);
   break;
   case 'O':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 15, 4, 16, 16);
   break;
   case 'P':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 0, 5, 16, 16);
   break;
   case 'Q':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 1, 5, 16, 16);
   break;
   case 'R':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 2, 5, 16, 16);
   break;
   case 'S':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 3, 5, 16, 16);
   break;
   case 'T':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 4, 5, 16, 16);
   break;
   case 'U':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 5, 5, 16, 16);
   break;
   case 'V':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 6, 5, 16, 16);
   break;
   case 'W':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 7, 5, 16, 16);
   break;
   case 'X':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 8, 5, 16, 16);
   break;
   case 'Y':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 9, 5, 16, 16);
   break;
   case 'Z':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 10, 5, 16, 16);
   break;
   case 'a':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 1, 6, 16, 16);
   break;
   case 'b':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 2, 6, 16, 16);
   break;
   case 'c':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 3, 6, 16, 16);
   break;
   case 'd':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 4, 6, 16, 16);
   break;
   case 'e':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 5, 6, 16, 16);
   break;
   case 'f':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 6, 6, 16, 16);
   break;
   case 'g':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 7, 6, 16, 16);
   break;
   case 'h':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 8, 6, 16, 16);
   break;
   case 'i':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 9, 6, 16, 16);
   break;
   case 'j':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 10, 6, 16, 16);
   break;
   case 'k':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 11, 6, 16, 16);
   break;
   case 'l':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 12, 6, 16, 16);
   break;
   case 'm':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 13, 6, 16, 16);
   break;
   case 'n':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 14, 6, 16, 16);
   break;
   case 'o':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 15, 6, 16, 16);
   break;
   case 'p':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 0, 7, 16, 16);
   break;
   case 'q':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 1, 7, 16, 16);
   break;
   case 'r':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 2, 7, 16, 16);
   break;
   case 's':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 3, 7, 16, 16);
   break;
   case 't':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 4, 7, 16, 16);
   break;
   case 'u':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 5, 7, 16, 16);
   break;
   case 'v':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 6, 7, 16, 16);
   break;
   case 'w':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 7, 7, 16, 16);
   break;
   case 'x':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 8, 7, 16, 16);
   break;
   case 'y':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 9, 7, 16, 16);
   break;
   case 'z':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 10, 7, 16, 16);
   break;
   case ':':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 10, 3, 16, 16);
   break;
   case '_':
        deadrsx_sprite(font_offset, x, y, size, size, 512, 512, 15, 5, 16, 16);
   break;
   }
   x += size/1.9;
 }

}
