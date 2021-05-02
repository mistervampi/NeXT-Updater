#include <stdio.h>
#include <string.h>
#include <switch.h>


#include "util.h"
#include "menu.h"
#include "unzip.h"
#include "download.h"
#include "reboot_payload.h"

#define TEMP_FILE                 "/switch/next-updater/temp"
#define FILTER_STRING             "browser_download_url\":\""
#define VERSION_FILTER_STRING     "tag_name\":\""

char g_sysVersion[50];
char g_amsVersion[50];
char g_versionNext[10];
char g_amsVersionWithoutHash[15];
char g_latestAtmosphereVersion[50];
char g_nextVer[50];


char *getSysVersion()
{
    return g_sysVersion;
}

char *getAmsVersion()
{
    return g_amsVersion;
}

char *getLatestAtmosphereVersion()
{
    return g_latestAtmosphereVersion;
}

char *getNextVersion()
{
    return g_nextVer;
}

void writeSysVersion()
{
	Result ret = 0;
	SetSysFirmwareVersion ver;

	if (R_FAILED(ret = setsysGetFirmwareVersion(&ver)))
    {
		printf("GetFirmwareVersion() failed: 0x%x.\n\n", ret);
		return;
	}

    char sysVersionBuffer[20];
	snprintf(sysVersionBuffer, 20, "%u.%u.%u", ver.major, ver.minor, ver.micro);
    snprintf(g_sysVersion, sizeof(g_sysVersion), "Horizon: %s", sysVersionBuffer);
}

void writeAmsVersion()
{
	Result ret = 0;
	u64 ver;
    u64 fullHash;
    SplConfigItem SplConfigItem_ExosphereVersion = (SplConfigItem)65000;
    SplConfigItem SplConfigItem_ExosphereVerHash = (SplConfigItem)65003;

	if (R_FAILED(ret = splGetConfig(SplConfigItem_ExosphereVersion, &ver)))
    {
		printf("SplConfigItem_ExosphereVersion() failed: 0x%x.\n\n", ret);
		return;
	}

    if (R_FAILED(ret = splGetConfig(SplConfigItem_ExosphereVerHash, &fullHash)))
    {
		printf("SplConfigItem_ExosphereVerHash() failed: 0x%x.\n\n", ret);
		return;
	}

    // write only the first 8 char of the hash.
    char shortHash[8];
	snprintf(shortHash, sizeof(shortHash), "%lx", fullHash);

    // write ams version number + hash.
    char amsVersionNum[25];
    snprintf(g_amsVersionWithoutHash, sizeof(g_amsVersionWithoutHash), "%lu.%lu.%lu", (ver >> 56) & 0xFF,  (ver >> 48) & 0xFF, (ver >> 40) & 0xFF);
	snprintf(amsVersionNum, sizeof(amsVersionNum), "%s (%s)", g_amsVersionWithoutHash, shortHash);

    // write string + ams version to global variable.
    snprintf(g_amsVersion, sizeof(g_amsVersion), "Atmosphere: %s", amsVersionNum);
}

void EscribeVersionNeXT() 
{
    FILE *fp;
    char str[10];
    char* filename = "/NEXT/NEXTV.txt";
    fp = fopen(filename, "r");
    while (fgets(str, 10, fp) != NULL)
    fclose(fp);
    snprintf(g_nextVer, sizeof(g_nextVer), "NeXT: %s", str);
    
    // Escribe Version de NeXT a variable global
    snprintf(g_versionNext, sizeof(g_versionNext), "%s" , str);
}

void writeLatestAtmosphereVersion()
{
  // Download the github API file and then parse out the version number.
  char *updateString = "- NeXT is up to date -";
  if (!downloadFile(AMS_URL, TEMP_FILE, ON))
  {
    char latestVersionNumber[10];
    if (!parseSearch(TEMP_FILE, VERSION_FILTER_STRING, latestVersionNumber)) {
      if (strcmp(g_versionNext, latestVersionNumber) != 0)
      {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "Update Available: %s", latestVersionNumber);
        updateString = buffer;
      }
    }
  }
  snprintf(g_latestAtmosphereVersion, sizeof(g_latestAtmosphereVersion), updateString);
}




void copyFile(char *src, char *dest)
{
    FILE *srcfile = fopen(src, "rb");
    FILE *newfile = fopen(dest, "wb");

    if (srcfile && newfile)
    {
        char buffer[10000]; // 10kb per write, which is fast
        size_t bytes;       // size of the file to write (10kb or filesize max)

        while (0 < (bytes = fread(buffer, 1, sizeof(buffer), srcfile)))
        {
            fwrite(buffer, 1, bytes, newfile);
        }
    }
    fclose(srcfile);
    fclose(newfile);
}

int parseSearch(char *parse_string, char *filter, char* new_string)
{
    FILE *fp = fopen(parse_string, "r");

    if (fp)
    {
        char c;
        while ((c = fgetc(fp)) != EOF)
        {
            if (c == *filter)
            {
                for (int i = 0, len = strlen(filter) - 1; c == filter[i]; i++)
                {
                    c = fgetc(fp);
                    if (i == len)
                    {
                        for (int j = 0; c != '\"'; j++)
                        {
                            new_string[j] = c;
                            new_string[j+1] = '\0';
                            c = fgetc(fp);
                        }
                        fclose(fp);
                        remove(parse_string);
                        return 0;
                    }
                }
            }
        }
    }

    errorBox(350, 250, "Failed to find parse url!");
    fclose(fp);
    return 1;
}

int update_ams_hekate(char *url, char *output, int mode)
{
    if (!downloadFile(url, TEMP_FILE, ON))
    {
        char new_url[MAX_STRLEN];
        if (!parseSearch(TEMP_FILE, FILTER_STRING, new_url))
        {
            if (!downloadFile(new_url, output, OFF))
            {
                unzip(output, mode);

                return 0;
            }
            return 1;
        }
        return 1;
    }
    return 1;
}
