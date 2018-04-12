/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include "stdafx.h"
#include <Project64-core/N64System/Mips/Eeprom.h>
#include <Project64-core/N64System/SystemGlobals.h>
#include <Project64-core/N64System/N64Class.h>
#include <time.h>

CEeprom::CEeprom(bool ReadOnly) :
m_ReadOnly(ReadOnly)
{
    memset(m_EEPROM, 0xFF, sizeof(m_EEPROM));
}

CEeprom::~CEeprom()
{
}

uint8_t byte2bcd(int32_t n)
{
    n %= 100;
    return (uint8_t)(((n / 10) << 4) | (n % 10));
}

void CEeprom::EepromCommand(uint8_t * Command)
{
    time_t curtime_time;
    struct tm curtime;

    if (g_System->m_SaveUsing == SaveChip_Auto)
    {
        g_System->m_SaveUsing = SaveChip_Eeprom_4K;
    }

    switch (Command[2])
    {
    case 0: // check
        if (g_System->m_SaveUsing != SaveChip_Eeprom_4K &&  g_System->m_SaveUsing != SaveChip_Eeprom_16K)
        {
            Command[1] |= 0x80;
            break;
        }
        if (Command[1] != 3)
        {
            Command[1] |= 0x40;
            if ((Command[1] & 3) > 0)
            {
                Command[3] = 0x00;
            }
            if ((Command[1] & 3) > 1)
            {
                Command[4] = (g_System->m_SaveUsing == SaveChip_Eeprom_4K) ? 0x80 : 0xC0;
            }
            if ((Command[1] & 3) > 2)
            {
                Command[5] = 0x00;
            }
        }
        else
        {
            Command[3] = 0x00;
            Command[4] = g_System->m_SaveUsing == SaveChip_Eeprom_4K ? 0x80 : 0xC0;
            Command[5] = 0x00;
        }
        break;
    case 4: // Read from Eeprom
        if (Command[0] != 2 && bHaveDebugger())
        {
            g_Notify->DisplayError("What am I meant to do with this Eeprom Command");
        }
        if (Command[1] != 8 && bHaveDebugger())
        {
            g_Notify->DisplayError("What am I meant to do with this Eeprom Command");
        }
        ReadFrom(&Command[4], Command[3]);
        break;
    case 5: //Write to Eeprom
        if (Command[0] != 10 && bHaveDebugger())
        {
            g_Notify->DisplayError("What am I meant to do with this Eeprom Command");
        }
        if (Command[1] != 1 && bHaveDebugger())
        {
            g_Notify->DisplayError("What am I meant to do with this Eeprom Command");
        }
        WriteTo(&Command[4], Command[3]);
        break;
    case 6: //RTC Status query
        Command[3] = 0x00;
        Command[4] = 0x10;
        Command[5] = 0x00;
        break;
    case 7: //Read RTC block
        switch (Command[3])
        {
        case 0: //Block number
            Command[4] = 0x00;
            Command[5] = 0x02;
            Command[12] = 0x00;
            break;
        case 1:
            //read block, Command[2], Unimplemented
            break;
        case 2: //Set RTC Time
            time(&curtime_time);
            memcpy(&curtime, localtime(&curtime_time), sizeof(curtime)); // fd's fix
            Command[4] = byte2bcd(curtime.tm_sec);
            Command[5] = byte2bcd(curtime.tm_min);
            Command[6] = 0x80 + byte2bcd(curtime.tm_hour);
            Command[7] = byte2bcd(curtime.tm_mday);
            Command[8] = byte2bcd(curtime.tm_wday);
            Command[9] = byte2bcd(curtime.tm_mon + 1);
            Command[10] = byte2bcd(curtime.tm_year);
            Command[11] = byte2bcd(curtime.tm_year / 100);
            Command[12] = 0x00;	// status
            break;
        }
        break;
    case 8:
        //Write RTC, unimplemented
        if (g_Settings->LoadDword(Debugger_ShowPifErrors))
        {
            g_Notify->DisplayError("Write RTC, unimplemented");
        }
        break;
    default:
        if (g_Settings->LoadDword(Debugger_ShowPifErrors))
        {
            g_Notify->DisplayError(stdstr_f("Unknown EepromCommand %d", Command[2]).c_str());
        }
    }
}

void CEeprom::LoadEeprom()
{
    memset(m_EEPROM, 0xFF, sizeof(m_EEPROM));

    CPath FileName(g_Settings->LoadStringVal(Directory_NativeSave).c_str(), stdstr_f("%s.eep", g_Settings->LoadStringVal(Game_GameName).c_str()).c_str());
    if (g_Settings->LoadBool(Setting_UniqueSaveDir))
    {
        FileName.AppendDirectory(g_Settings->LoadStringVal(Game_UniqueSaveDir).c_str());
    }

    if (!FileName.DirectoryExists())
    {
        FileName.DirectoryCreate();
    }

    if (!m_File.Open(FileName, (m_ReadOnly ? CFileBase::modeRead : CFileBase::modeReadWrite) | CFileBase::modeNoTruncate | CFileBase::modeCreate))
    {
#ifdef _WIN32
        WriteTrace(TraceN64System, TraceError, "Failed to open (%s), ReadOnly = %d, LastError = %X", (const char *)FileName, m_ReadOnly, GetLastError());
#else
        WriteTrace(TraceN64System, TraceError, "Failed to open (%s), ReadOnly = %d", (const char *)FileName, m_ReadOnly);
#endif
        g_Notify->DisplayError(GS(MSG_FAIL_OPEN_EEPROM));
        return;
    }
    m_File.SeekToBegin();
    m_File.Read(m_EEPROM, sizeof(m_EEPROM));
}

void CEeprom::ReadFrom(uint8_t * Buffer, int32_t line)
{
    int32_t i;

    if (!m_File.IsOpen())
    {
        LoadEeprom();
    }

    for (i = 0; i < 8; i++)
    {
        Buffer[i] = m_EEPROM[line * 8 + i];
    }
}

void CEeprom::WriteTo(uint8_t * Buffer, int32_t line)
{
    int32_t i;

    if (!m_File.IsOpen())
    {
        LoadEeprom();
    }
    for (i = 0; i < 8; i++)
    {
        m_EEPROM[line * 8 + i] = Buffer[i];
    }
    if (!m_ReadOnly)
    {
        m_File.Seek(line * 8, CFile::begin);
        m_File.Write(Buffer, 8);
    }
}