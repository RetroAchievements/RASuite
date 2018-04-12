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
#include <Project64-core/N64System/Recompiler/FunctionInfo.h>

CCompiledFunc::CCompiledFunc( const CCodeBlock & CodeBlock ) :
    m_EnterPC(CodeBlock.VAddrEnter()),
    m_MinPC(CodeBlock.VAddrFirst()),
    m_MaxPC(CodeBlock.VAddrLast()),
    m_Hash(CodeBlock.Hash()),
    m_Function((Func)CodeBlock.CompiledLocation()),
    m_FunctionEnd(CodeBlock.CompiledLocationEnd()),
    m_Next(NULL)
{
    m_MemContents[0] = CodeBlock.MemContents(0);
    m_MemContents[1] = CodeBlock.MemContents(1);
    m_MemLocation[0] = CodeBlock.MemLocation(0);
    m_MemLocation[1] = CodeBlock.MemLocation(1);

#if defined(__arm__) || defined(_M_ARM)
    // make sure function starts at odd address so that the system knows it is thumb mode
    if ((((uint32_t)m_Function) % 2) == 0)
    {
        m_Function = (Func)(((uint32_t)m_Function) + 1);
    }
#endif
}