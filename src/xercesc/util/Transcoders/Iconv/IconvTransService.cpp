/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Id$
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include "IconvTransService.hpp"
#include <wchar.h>
#if defined (XML_GCC) || defined (XML_PTX) || defined (XML_IBMVAOS2)
    #if defined(XML_BEOS)
        wint_t towlower(wint_t wc) {
          return ((wc>'A')&&(wc<'Z') ? wc+'a'-'A' : wc);
        }
        wint_t towupper(wint_t wc) {
          return ((wc>'a')&&(wc<'z') ? wc-'a'+'A' : wc);
        }
        wint_t iswspace(wint_t wc) {
          return (wc==(wint_t)' ');
        }
    #elif !defined(XML_OPENSERVER)
        #include <wctype.h>
    #endif
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local, const data
// ---------------------------------------------------------------------------
static const int    gTempBuffArraySize = 1024;
static const XMLCh  gMyServiceId[] =
{
    chLatin_I, chLatin_C, chLatin_o, chLatin_n, chLatin_v, chNull
};


// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
static unsigned int getWideCharLength(const XMLCh* const src)
{
    if (!src)
        return 0;

    unsigned int len = 0;
    const XMLCh* pTmp = src;
    while (*pTmp++)
        len++;
    return len;
}



// ---------------------------------------------------------------------------
//  IconvTransService: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvTransService::IconvTransService()
{
}

IconvTransService::~IconvTransService()
{
}


// ---------------------------------------------------------------------------
//  IconvTransService: The virtual transcoding service API
// ---------------------------------------------------------------------------
int IconvTransService::compareIString(  const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2)
{
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    while ( (*cptr1 != 0) && (*cptr2 != 0) )
    {
        wint_t wch1 = towupper(*cptr1);
        wint_t wch2 = towupper(*cptr2);
        if (wch1 != wch2)
            break;

        cptr1++;
        cptr2++;
    }
    return (int) ( towupper(*cptr1) - towupper(*cptr2) );
}


int IconvTransService::compareNIString( const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2
                                        , const unsigned int    maxChars)
{
    unsigned int  n = 0;
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    while (true && maxChars)
    {
        wint_t wch1 = towupper(*cptr1);
        wint_t wch2 = towupper(*cptr2);

        if (wch1 != wch2)
            return (int) (wch1 - wch2);

        // If either ended, then both ended, so equal
        if (!*cptr1 || !*cptr2)
            break;

        cptr1++;
        cptr2++;

        //  Bump the count of chars done. If it equals the count then we
        //  are equal for the requested count, so break out and return
        //  equal.
        n++;
        if (n == maxChars)
            break;
    }

    return 0;
}


const XMLCh* IconvTransService::getId() const
{
    return gMyServiceId;
}


bool IconvTransService::isSpace(const XMLCh toCheck) const
{
    return (iswspace(toCheck) != 0);
}


XMLLCPTranscoder* IconvTransService::makeNewLCPTranscoder()
{
    // Just allocate a new transcoder of our type
    return new IconvLCPTranscoder;
}

bool IconvTransService::supportsSrcOfs() const
{
    return true;
}


// ---------------------------------------------------------------------------
//  IconvTransService: The protected virtual transcoding service API
// ---------------------------------------------------------------------------
XMLTranscoder*
IconvTransService::makeNewXMLTranscoder(const   XMLCh* const            encodingName
                                        ,       XMLTransService::Codes& resValue
                                        , const unsigned int            )
{
    //
    //  NOTE: We don't use the block size here
    //
    //  This is a minimalist transcoding service, that only supports a local
    //  default transcoder. All named encodings return zero as a failure,
    //  which means that only the intrinsic encodings supported by the parser
    //  itself will work for XML data.
    //
    resValue = XMLTransService::UnsupportedEncoding;
    return 0;
}

void IconvTransService::upperCase(XMLCh* const toUpperCase) const
{
    XMLCh* outPtr = toUpperCase;
    while (*outPtr)
    {
        *outPtr = towupper(*outPtr);
        outPtr++;
    }
}

void IconvTransService::lowerCase(XMLCh* const toLowerCase) const
{
    XMLCh* outPtr = toLowerCase;
    while (*outPtr)
    {
        *outPtr = towlower(*outPtr);
        outPtr++;
    }
}


// ---------------------------------------------------------------------------
//  IconvLCPTranscoder: The virtual transcoder API
// ---------------------------------------------------------------------------
unsigned int IconvLCPTranscoder::calcRequiredSize(const char* const srcText)
{
    if (!srcText)
        return 0;

    const unsigned int retVal = ::mbstowcs(NULL, srcText, 0);

    if (retVal == ~0)
        return 0;
    return retVal;
}


unsigned int IconvLCPTranscoder::calcRequiredSize(const XMLCh* const srcText)
{
    if (!srcText)
        return 0;

    unsigned int  wLent = getWideCharLength(srcText);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (wLent >= gTempBuffArraySize)
        wideCharBuf = allocatedArray = new wchar_t[wLent + 1];
    else
        wideCharBuf = tmpWideCharArr;

    for (unsigned int i = 0; i < wLent; i++)
    {
        wideCharBuf[i] = srcText[i];
    }
    wideCharBuf[wLent] = 0x00;

    const unsigned int retVal = ::wcstombs(NULL, wideCharBuf, 0);
    delete [] allocatedArray;

    if (retVal == ~0)
        return 0;
    return retVal;
}


char* IconvLCPTranscoder::transcode(const XMLCh* const toTranscode)
{
    if (!toTranscode)
        return 0;

    char* retVal = 0;
    if (*toTranscode)
    {
        unsigned int  wLent = getWideCharLength(toTranscode);

        wchar_t       tmpWideCharArr[gTempBuffArraySize];
        wchar_t*      allocatedArray = 0;
        wchar_t*      wideCharBuf = 0;

        if (wLent >= gTempBuffArraySize)
            wideCharBuf = allocatedArray = new wchar_t[wLent + 1];
        else
            wideCharBuf = tmpWideCharArr;

        for (unsigned int i = 0; i < wLent; i++)
        {
            wideCharBuf[i] = toTranscode[i];
        }
        wideCharBuf[wLent] = 0x00;

        // Calc the needed size.
        const size_t neededLen = ::wcstombs(NULL, wideCharBuf, 0);
        if (neededLen == -1)
        {
            delete [] allocatedArray;
            retVal = new char[1];
            retVal[0] = 0;
            return retVal;
        }

        retVal = new char[neededLen + 1];
        ::wcstombs(retVal, wideCharBuf, neededLen);
        retVal[neededLen] = 0;
        delete [] allocatedArray;
    }
    else
    {
        retVal = new char[1];
        retVal[0] = 0;
    }
    return retVal;
}


bool IconvLCPTranscoder::transcode( const   XMLCh* const    toTranscode
                                    ,       char* const     toFill
                                    , const unsigned int    maxBytes)
{
    // Watch for a couple of pyscho corner cases
    if (!toTranscode || !maxBytes)
    {
        toFill[0] = 0;
        return true;
    }

    if (!*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

    unsigned int  wLent = getWideCharLength(toTranscode);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (wLent > maxBytes) {
        wLent = maxBytes;
    }

    if (maxBytes >= gTempBuffArraySize)
        wideCharBuf = allocatedArray = new wchar_t[maxBytes + 1];
    else
        wideCharBuf = tmpWideCharArr;

    for (unsigned int i = 0; i < wLent; i++)
    {
        wideCharBuf[i] = toTranscode[i];
    }
    wideCharBuf[wLent] = 0x00;

    // Ok, go ahead and try the transcoding. If it fails, then ...
    size_t mblen ::wcstombs(toFill, wideCharBuf, maxBytes);
    if (mblen == -1)
    {
        delete [] allocatedArray;
        return false;
    }

    // Cap it off just in case
    toFill[mblen] = 0;
    delete [] allocatedArray;
    return true;
}



XMLCh* IconvLCPTranscoder::transcode(const char* const toTranscode)
{
    if (!toTranscode)
        return 0;

    XMLCh* retVal = 0;
    if (*toTranscode)
    {
        const unsigned int len = calcRequiredSize(toTranscode);
        if (len == 0)
        {
            retVal = new XMLCh[1];
            retVal[0] = 0;
            return retVal;
        }

        wchar_t       tmpWideCharArr[gTempBuffArraySize];
        wchar_t*      allocatedArray = 0;
        wchar_t*      wideCharBuf = 0;

        if (len >= gTempBuffArraySize)
            wideCharBuf = allocatedArray = new wchar_t[len + 1];
        else
            wideCharBuf = tmpWideCharArr;

        ::mbstowcs(wideCharBuf, toTranscode, len);
        retVal = new XMLCh[len + 1];
        for (unsigned int i = 0; i < len; i++)
        {
            retVal[i] = (XMLCh) wideCharBuf[i];
        }
        retVal[len] = 0x00;
        delete [] allocatedArray;
    }
    else
    {
        retVal = new XMLCh[1];
        retVal[0] = 0;
    }
    return retVal;
}


bool IconvLCPTranscoder::transcode( const   char* const     toTranscode
                                    ,       XMLCh* const    toFill
                                    , const unsigned int    maxChars)
{
    // Check for a couple of psycho corner cases
    if (!toTranscode || !maxChars)
    {
        toFill[0] = 0;
        return true;
    }

    if (!*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

    unsigned int len = calcRequiredSize(toTranscode);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (len > maxChars) {
        len = maxChars;
    }

    if (maxChars >= gTempBuffArraySize)
        wideCharBuf = allocatedArray = new wchar_t[maxChars + 1];
    else
        wideCharBuf = tmpWideCharArr;

    if (::mbstowcs(wideCharBuf, toTranscode, maxChars) == -1)
    {
        delete [] allocatedArray;
        return false;
    }

    for (unsigned int i = 0; i < len; i++)
    {
        toFill[i] = (XMLCh) wideCharBuf[i];
    }
    toFill[len] = 0x00;
    delete [] allocatedArray;
    return true;
}



// ---------------------------------------------------------------------------
//  IconvLCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvLCPTranscoder::IconvLCPTranscoder()
{
}

IconvLCPTranscoder::~IconvLCPTranscoder()
{
}

// ---------------------------------------------------------------------------
//  IconvTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvTranscoder::IconvTranscoder(const  XMLCh* const    encodingName
                                , const unsigned int    blockSize) :

    XMLTranscoder(encodingName, blockSize)
{
}

IconvTranscoder::~IconvTranscoder()
{
}


// ---------------------------------------------------------------------------
//  IconvTranscoder: Implementation of the virtual transcoder API
// ---------------------------------------------------------------------------
XMLCh IconvTranscoder::transcodeOne(const   XMLByte* const  srcData
                                    , const unsigned int    srcBytes
                                    ,       unsigned int&   bytesEaten)
{
    wchar_t  toFill;
    int eaten = ::mbtowc(&toFill, (const char*)srcData, srcBytes);
    if (eaten == -1)
    {
        bytesEaten = 0;
        return 0;
    }

    // Return the bytes we ate and the resulting char.
    bytesEaten = eaten;
    return toFill;
}


unsigned int
IconvTranscoder::transcodeXML(  const   XMLByte* const          srcData
                                , const unsigned int            srcCount
                                ,       XMLCh* const            toFill
                                , const unsigned int            maxChars
                                ,       unsigned int&           bytesEaten
                                ,       unsigned char* const    charSizes)
{
    //
    //  For this one, because we have to maintain the offset table, we have
    //  to do them one char at a time until we run out of source data.
    //
    unsigned int countIn = 0;
    unsigned int countOut = 0;
    while (countOut < maxChars)
    {
        wchar_t   oneWideChar;
        const int bytesEaten =
            ::mbtowc(&oneWideChar, (const char*)&srcData[countIn], srcCount - countIn);

        // We are done, so break out
        if (bytesEaten == -1)
            break;
        toFill[countOut] = (XMLCh) oneWideChar;
        countIn += (unsigned int) bytesEaten;
        countOut++;
    }

    // Give back the counts of eaten and transcoded
    bytesEaten = countIn;
    return countOut;
}

XERCES_CPP_NAMESPACE_END
