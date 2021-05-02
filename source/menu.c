#include <unistd.h>
#include <switch.h>
#include "sdl.h"
#include "menu.h"
#include "touch.h"
#include "util.h"

#define APP_VERSION "NeXT Updater: 1.0.4"



void refreshScreen(char loaded)
{
    clearRenderer();

    // app version.
    drawText(appFonts.fntLarge, 400, 40, SDL_GetColour(white), APP_VERSION);

    // system version.
    drawText(appFonts.fntSmall, 25, 150, SDL_GetColour(white), getSysVersion());

    // atmosphere version.
    drawText(appFonts.fntSmall, 25, 180, SDL_GetColour(white), getAmsVersion());

    if (loaded)
    {

      // write the latest version number, if an update is available
      drawText(appFonts.fntSmall, 25, 100, SDL_GetColour(white), "Current Versions on System:");
      drawText(appFonts.fntLarge, 600, 450, SDL_GetColour(white), getLatestAtmosphereVersion());
      
      // Escribe la versión de NeXT actual
      drawText(appFonts.fntSmall, 25, 215, SDL_GetColour(white), getNextVersion());

      //drawText(fntMedium, 120, 225, SDL_GetColour(white), "Menu Here"); // menu options
      drawButton(appFonts.fntButton, BUTTON_A, 970, 672, SDL_GetColour(white));
      drawText(appFonts.fntSmall, 1010, 675, SDL_GetColour(white), "Select");

      drawButton(appFonts.fntButton, BUTTON_PLUS, 1145, 672, SDL_GetColour(white));
      drawText(appFonts.fntSmall, 1185, 675, SDL_GetColour(white), "Exit");
    }
}

void printOptionList(int cursor)
{
    refreshScreen(/*loaded=*/1);

    char *option_list[]      = {    "Update NeXT", \
                                    "Restart and apply update" };

    char *description_list[] = {    "Install the most recent version of NeXT", \
                                    "Restarts console and apply changes." };

    SDL_Texture *textureArray[] = { appTextures.ams_icon, appTextures.reboot_icon };

    for (int i=0, nl=0; i < (CURSOR_LIST_MAX+1); i++, nl+=NEWLINE)
    {
        if (cursor != i) drawText(appFonts.fntSmall, 550, FIRST_LINE+nl, SDL_GetColour(white), option_list[i]);
        else
        {
            // icon for the option selected.
            drawImage(textureArray[i], 125, 350);
            // highlight box.
            drawShape(SDL_GetColour(dark_blue), 530, (FIRST_LINE + nl - HIGHLIGHT_BOX_MIN), 700, HIGHLIGHT_BOX_MAX);
            // option text.
            drawText(appFonts.fntSmall, 550, FIRST_LINE+nl, SDL_GetColour(jordy_blue), option_list[i]);
            // description.
            drawText(appFonts.fntSmall, 25, 675, SDL_GetColour(white), description_list[i]);
        }
    }
}

void popUpBox(TTF_Font *font, int x, int y, SDL_Colour colour, char *text)
{
    // outline. box
    drawShape(SDL_GetColour(black), (SCREEN_W/4)-5, (SCREEN_H/4)-5, (SCREEN_W/2)+10, (SCREEN_H/2)+10);
    // popup box.
    drawShape(SDL_GetColour(dark_blue), SCREEN_W/4, SCREEN_H/4, SCREEN_W/2, SCREEN_H/2);
    // text to draw.
    drawText(font, x, y, colour, text);
}

int yesNoBox(int mode, int x, int y, char *question)
{
    printOptionList(mode);
    popUpBox(appFonts.fntMedium, x, y, SDL_GetColour(white), question);
    // highlight box.
    drawShape(SDL_GetColour(faint_blue), 380, 410, 175, 65);
    drawShape(SDL_GetColour(faint_blue), 700, 410, 190, 65);
    // option text.
    drawButton(appFonts.fntButtonBig, BUTTON_B, 410, 425, SDL_GetColour(white));
    drawText(appFonts.fntMedium, 455, 425, SDL_GetColour(white), "No");
    drawButton(appFonts.fntButtonBig, BUTTON_A, 725, 425, SDL_GetColour(white));
    drawText(appFonts.fntMedium, 770, 425, SDL_GetColour(white), "Yes");

    updateRenderer();

    int res = 0;
    int touch_lock = OFF;
    touchPosition touch;
    u32 tch = 0;
    u32 touch_count = hidTouchCount();

    // check if the user is already touching the screen.
    if (touch_count > 0) touch_lock = ON;

    while (1)
    {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        hidTouchRead(&touch, tch);
        touch_count = hidTouchCount();

        if (touch_count == 0) touch_lock = OFF;

        if (touch_count > 0 && touch_lock == OFF)
            res = touch_yes_no_option(touch.px, touch.py);

        if (kDown & KEY_A || res == YES)
            return YES;

        if (kDown & KEY_B || res == NO)
            return NO;
    }
}

void errorBox(int x, int y, char *errorText)
{
    popUpBox(appFonts.fntMedium, x, y, SDL_GetColour(white), errorText);
    drawImageScale(appTextures.error_icon, 570, 340, 128, 128);
    updateRenderer();

    sleep(3);
}
