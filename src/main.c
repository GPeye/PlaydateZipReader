#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
#include "unzip.h"

static int update(void *userdata);
const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont *font = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif

	static PlaydateAPI *pd = NULL;


void *myOpen(const char *filename, int32_t *size)
{
    pd->system->logToConsole("Attempting to open");
    size_t filesize;
    SDFile *myfile = pd->file->open(filename, 1);
    if (myfile)
    {
        pd->file->seek(myfile, 0, SEEK_END);
        filesize = pd->file->tell(myfile);
        pd->file->seek(myfile, 0, SEEK_SET);
        *size = (int32_t)filesize;
    }
    pd->system->logToConsole("myfile handle: %d", myfile);
    return (void *)myfile;
}

void myClose(void *p)
{
    ZIPFILE *pzf = (ZIPFILE *)p;
    SDFile *f = (SDFile *)pzf->fHandle;
    pd->file->close(f);
    pd->system->logToConsole("File closed on handle: %d", f);
}

int32_t myRead(void *p, uint8_t *buffer, int32_t length)
{
    ZIPFILE *pzf = (ZIPFILE *)p;
    if (!pzf)
        return 0;
    SDFile *f = (SDFile *)pzf->fHandle;
    return (int32_t)pd->file->read(f, buffer, length);
}

int32_t mySeek(void *p, int32_t position, int iType)
{
    ZIPFILE *pzf = (ZIPFILE *)p;
    if (!pzf)
        return 0;
    SDFile *f = (SDFile *)pzf->fHandle;
    return pd->file->seek(f, position, iType);
}

void readStuff(char *zipfilename, char *contentname)
{
    // listFiles(pd);
    pd->system->logToConsole("reading files");
    int rc = 0;
    ZIPFILE zpf;
    unzFile zHandle = unzOpen(zipfilename, NULL, 0, &zpf, myOpen, myRead, mySeek, myClose);

    if (zHandle == NULL)
    {
        pd->system->logToConsole("Failed to open");
    }
    else
    {
        pd->system->logToConsole("We got a handle: %d", zHandle);
    }
    char szComment[256];
    rc = unzGetGlobalComment(zHandle, szComment, sizeof(szComment));
    if (rc == UNZ_OK)
    {
        pd->system->logToConsole("comment: %s", &szComment);
    }
    else
    {
        pd->system->logToConsole("bad comment %d", rc);
    }

    rc = unzLocateFile(zHandle, contentname, 2);
    //rc = unzLocateFile(zHandle, "testfile", 2);

    if (rc != UNZ_OK) /* Report the file not found */
    {
        pd->system->logToConsole("file %s not found within archive", contentname);
        unzClose(zHandle);
        //return -1;
    }
    else
    {
        pd->system->logToConsole("file found");
        rc = unzOpenCurrentFile(zHandle); /* Try to open the file we want */
        if (rc != UNZ_OK)
        {
            pd->system->logToConsole("Error opening file = %d\n", rc);
            unzClose(zHandle);
            //return -1;
        }
        pd->system->logToConsole("File located within archive.\n");
        rc = 1;
        int i = 0;

        char szTemp[256];
        while (rc > 0 && i <= 1024) // just read the first 1024 bytes
        {
            rc = unzReadCurrentFile(zHandle, szTemp, sizeof(szTemp));
            if (rc >= 0)
            {
                i += rc;
                if (rc > 0)
                {
                    pd->system->logToConsole("%s", szTemp);
                }
            }
            else
            {
                pd->system->logToConsole("Error reading from file\n");
                break;
            }
        }
        pd->system->logToConsole("Total bytes read = %d (reading 256 bytes at a time)\n", i);
        rc = unzCloseCurrentFile(zHandle);
        unzClose(zHandle);
    }
}


int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if (event == kEventInit)
	{
		pd = playdate;
		const char *err;
		font = pd->graphics->loadFont(fontpath, &err);
		pd->display->setRefreshRate(30);

		if (font == NULL)
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		// Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);

		//readStuff("testfile.zip","testfile");
		readStuff("moby.epub","OEBPS/3484760691463238453_2701-h-1.htm.html");
	}

	return 0;
}

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

int x = (400 - TEXT_WIDTH) / 2;
int y = (240 - TEXT_HEIGHT) / 2;
int dx = 5;
int dy = 7;

static int update(void *userdata)
{
	PlaydateAPI *pd = userdata;
	
	pd->graphics->fillRect(x, y,TEXT_WIDTH, TEXT_HEIGHT, kColorWhite);

	x += dx;
	y += dy;

	if (x < 0 || x > LCD_COLUMNS - TEXT_WIDTH)
		dx = -dx;

	if (y < 0 || y > LCD_ROWS - TEXT_HEIGHT)
		dy = -dy;

	//pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);
	pd->graphics->drawText("Hello World!", strlen("Hello World!"), kASCIIEncoding, x, y);


	pd->system->drawFPS(0, 0);

	return 1;
}
