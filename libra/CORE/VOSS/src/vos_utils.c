/*============================================================================
  FILE:         vos_utils.c

  OVERVIEW:     This source file contains definitions for vOS crypto APIs
                The four APIs mentioned in this file are used for 
                initializing, and de-initializing a crypto context, and
                obtaining truly random data (for keys), as well as
                SHA1 HMAC, and AES encrypt and decrypt routines.

                The routines include: 
                vos_crypto_init() - Initializes Crypto module
                vos_crypto_deinit() - De-initializes Crypto module
                vos_rand_get_bytes() - Generates random byte
                vos_sha1_hmac_str() - Generate the HMAC-SHA1 of a string given a key
                vos_encrypt_AES() - Generate AES Encrypted byte stream
                vos_decrypt_AES() - Decrypts an AES Encrypted byte stream

  DEPENDENCIES: 
 
                Copyright (c) 2007 QUALCOMM Incorporated.
                All Rights Reserved.
                Qualcomm Confidential and Proprietary
============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/

#include "vos_trace.h"
#include "vos_utils.h"
#include "sha.h"
#include "aes.h"
#include "vos_memory.h"
//#if 0
#include <linux/random.h>
//#endif
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/
#if 0
HCRYPTPROV hCryptProv;
#endif

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
   Function Definitions and Documentation
 * -------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
  
  \brief vos_crypto_init() - Initializes Crypto module
  
  The vos_crypto_init() function initializes Crypto module.

  \param phCryptProv - pointer to the Crypt handle
  
  \return VOS_STATUS_SUCCESS - Successfully generated random memory.

          VOS_STATUS_E_FAULT  - pbBuf is an invalid pointer.   

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons

       ***VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable
  \sa
   
    ( *** return value not considered yet )
  --------------------------------------------------------------------------*/
VOS_STATUS vos_crypto_init( v_U32_t *phCryptProv )
{
    VOS_STATUS uResult = VOS_STATUS_E_FAILURE;

    // This implementation doesn't require a crypto context
    *phCryptProv  = 0;
	uResult = VOS_STATUS_SUCCESS;
    return ( uResult );
}

VOS_STATUS vos_crypto_deinit( v_U32_t hCryptProv )
{
    VOS_STATUS uResult = VOS_STATUS_E_FAILURE;

    // CryptReleaseContext succeeded
	uResult = VOS_STATUS_SUCCESS;

    return ( uResult );
}

/*--------------------------------------------------------------------------
  
  \brief vos_rand_get_bytes() - Generates random byte
  
  The vos_rand_get_bytes() function generate random bytes.

  Buffer should be allocated before calling vos_rand_get_bytes().
  
  Attempting to initialize an already initialized lock results in 
  a failure.
 
  \param lock - pointer to the opaque lock object to initialize
  
  \return VOS_STATUS_SUCCESS - Successfully generated random memory.

          VOS_STATUS_E_FAULT  - pbBuf is an invalid pointer.   

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons

       ***VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable
  \sa
   
    ( *** return value not considered yet )
  --------------------------------------------------------------------------*/
VOS_STATUS vos_rand_get_bytes( v_U32_t cryptHandle, v_U8_t *pbBuf, v_U32_t numBytes )
{
   VOS_STATUS uResult = VOS_STATUS_E_FAILURE;
   //v_UINT_t uCode;
//   HCRYPTPROV hCryptProv = (HCRYPTPROV) cryptHandle;	

   //check for invalid pointer
   if ( NULL == pbBuf )
   {
      uResult = VOS_STATUS_E_FAULT; 
      return ( uResult );
   }

//#if 0
   // get_random_bytes() is a void procedure
   get_random_bytes( pbBuf, numBytes); 
   // "Random sequence generated."
   uResult = VOS_STATUS_SUCCESS;
//#endif

   return ( uResult );
}


/*--------------------------------------------------------------------------
  
  \brief vos_sha1_hmac_str() - Generate the HMAC-SHA1 of a string given a key
  
  The vos_sha1_hmac_str() function generate the HMAC-SHA1 of a string given a key.

  Buffer should be allocated before calling vos_rand_get_bytes().
  
  Attempting to initialize an already initialized lock results in 
  a failure.
 
  \param lock - pointer to the opaque lock object to initialize
  
  \return VOS_STATUS_SUCCESS - Successfully generated random memory.

          VOS_STATUS_E_FAULT  - pbBuf is an invalid pointer.   

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons

       ***VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable
  \sa
   
    ( *** return value not considered yet )
  --------------------------------------------------------------------------*/


#if 0
typedef struct {
    BLOBHEADER hdr;
    DWORD cbKeySize;
    BYTE rgbKeyData [];
} PlainTextKey;
#endif



VOS_STATUS vos_sha1_hmac_str(v_U32_t cryptHandle, /* Handle */
           v_U8_t *pText, /* pointer to data stream */
           v_U32_t textLen, /* length of data stream */
           v_U8_t *pKey, /* pointer to authentication key */
           v_U32_t keyLen, /* length of authentication key */
           v_U8_t digest[VOS_DIGEST_SHA1_SIZE])/* caller digest to be filled in */
{

    SHA_CTX context;
    unsigned char k_ipad[65];    /* inner padding -
                                  * key XORd with ipad
                                  */
    unsigned char k_opad[65];    /* outer padding -
                                  * key XORd with opad
                                  */
    unsigned char tk[VOS_DIGEST_SHA1_SIZE];
#if 0    
    /**Test vectors**/
    SHA_CTX testCtx;
    unsigned char testData[] = "The quick brown fox jumps over the lazy dog" ;
    v_U8_t tDigest[VOS_DIGEST_SHA1_SIZE];
    int tcnt = 0;
    /** End of test vectors **/
#endif
    int i;
    /* if key is longer than 64 bytes reset it to key=SHA1(key) */
    if (keyLen > 64) {
        
        SHA_CTX      tctx;
        
        SHA1_Init(&tctx);
        SHA1_Update(&tctx, pKey, keyLen);
        SHA1_Final(tk, &tctx);
        
        pKey = tk;
        keyLen = VOS_DIGEST_SHA1_SIZE;
    }
    
    /*
     * the HMAC_SHA1 transform looks like:
     *
     * SHA1(K XOR opad, SHA1(K XOR ipad, text))
     *
     * where K is an n byte key
     * ipad is the byte 0x36 repeated 64 times
     * opad is the byte 0x5c repeated 64 times
     * and text is the data being protected
     */
    
    /* start out by storing key in pads */
    memset( k_ipad, 0, sizeof k_ipad);
    memset( k_opad, 0, sizeof k_opad);
    memcpy((void *)k_ipad, (void *)pKey, keyLen );
    memcpy((void *)k_opad, (void *)pKey, keyLen);
    
    /* XOR key with ipad and opad values */
    for (i=0; i<64; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    /*
     * perform inner SHA1
     */
     SHA1_Init(&context);   /* init context for 1st
                           * pass */
    SHA1_Update(&context, k_ipad, 64);      /* start with inner pad */
    SHA1_Update(&context, pText, textLen); /* then text of datagram */
    SHA1_Final(digest, &context);  /* finish up 1st pass */

#if 0    
    /**Test Vectors**/
    SHA1_Init(&testCtx);   /* init context for 1st
                           * pass */
//    SHA1_Update(&testCtx, k_ipad, 64);      /* start with inner pad */
    SHA1_Update(&testCtx, "The quick brown fox jumps over the lazy dog", 43/*sizeof(testData)*/); /* then text of datagram */
    SHA1_Final(tDigest, &testCtx);  /* finish up 1st pass */
    for(tcnt = 0; tcnt < VOS_DIGEST_SHA1_SIZE; tcnt++){
    VOS_TRACE(VOS_MODULE_ID_VOSS,VOS_TRACE_LEVEL_ERROR,"tDigest[%d]=%x \n",tcnt,tDigest[tcnt]);}
    /*** End of test vecxtors **/
#endif
    
    
    /*
     * perform outer SHA1
     */
    SHA1_Init(&context);   /* init context for 2nd
                           * pass */
    SHA1_Update(&context, k_opad, 64);     /* start with outer pad */
    SHA1_Update(&context, digest, VOS_DIGEST_SHA1_SIZE);     /* then results of 1st
                                           * hash */
    SHA1_Final(digest, &context);  /* finish up 2nd pass */
    return VOS_STATUS_SUCCESS;
#if 0
   BOOL status;
   v_UINT_t uCode;

   size_t nBuffer = 0;
   LPBYTE pBuffer = NULL;
   PlainTextKey *pBlob = NULL;

   HCRYPTKEY hKey;
   HCRYPTHASH hHash;
   HCRYPTPROV hCryptProv = (HCRYPTPROV) cryptHandle;
   HMAC_INFO info;
   DWORD cbData = ( DWORD ) textLen;
   DWORD cbMac;

   __try
   {
      //check for invalid pointer
      if ( NULL == pText || NULL == pKey || hCryptProv == NULL)
      {
         //VOS_ASSERT ( NULL == lock ); 
         uResult = VOS_STATUS_E_FAULT; 
         __leave;
      }

      nBuffer = sizeof( PlainTextKey ) + keyLen;
      pBuffer = vos_mem_malloc( nBuffer );
      vos_mem_zero( pBuffer, nBuffer );
      pBlob = ( PlainTextKey* ) ( LPBYTE ) pBuffer;

      pBlob->hdr.bType    = PLAINTEXTKEYBLOB;
      pBlob->hdr.bVersion = CUR_BLOB_VERSION;
      pBlob->hdr.reserved = 0;
      pBlob->hdr.aiKeyAlg = ALG_CLASS_DATA_ENCRYPT | ALG_TYPE_BLOCK |
       ALG_SID_RC2;
      //   WLANCXXUTIL_ASSERT( DWORD_MAX >= keyLen );
      pBlob->cbKeySize = ( DWORD ) keyLen;
	  vos_mem_copy( pBlob + 1, pKey, keyLen );

      // That done, we import the key:
      status = CryptImportKey( hCryptProv, pBuffer, nBuffer, 0,
                               CRYPT_IPSEC_HMAC_KEY, &hKey );

      status = CryptCreateHash( hCryptProv, CALG_HMAC, hKey,
                               0, &hHash );

	  vos_mem_zero( &info, sizeof( info ) );
      info.HashAlgid = CALG_SHA1;
      status = CryptSetHashParam( hHash, HP_HMAC_INFO,
                                  ( const BYTE* ) &info, 0 );

      //    WLANCXXUTIL_ASSERT( DWORD_MAX >= textLen );
      status = CryptHashData( hHash, pText, cbData, 0 );

      cbData = sizeof( DWORD );
      status = CryptGetHashParam( hHash, HP_HASHSIZE, ( BYTE* ) &cbMac,
                                  &cbData, 0 );

      if ( cbMac > VOS_DIGEST_SHA1_SIZE )
      {
            //  Error : OutputBufferTooSmall
      }

      status = CryptGetHashParam( hHash, HP_HASHVAL, digest, &cbMac, 0 );

      uResult = VOS_STATUS_SUCCESS;
   }
   __except( uCode = GetExceptionCode() )
   {
      if ( STATUS_NO_MEMORY == uCode )	
      {
         uResult = VOS_STATUS_E_NOMEM;
      }
   }

#endif
}


/*--------------------------------------------------------------------------
  
  \brief vos_encrypt_AES() - Generate AES Encrypted byte stream
  
  The vos_encrypt_AES() function generates the encrypted byte stream for given text.

  Buffer should be allocated before calling vos_rand_get_bytes().
  
  Attempting to initialize an already initialized lock results in 
  a failure.
 
  \param lock - pointer to the opaque lock object to initialize
  
  \return VOS_STATUS_SUCCESS - Successfully generated random memory.

          VOS_STATUS_E_FAULT  - pbBuf is an invalid pointer.   

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons

       ***VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable
  \sa
   
    ( *** return value not considered yet )
  --------------------------------------------------------------------------*/

#define IV_SIZE_AES_128 16
#define KEY_SIZE_AES_128 16
#define AES_BLOCK_SIZE 16

VOS_STATUS vos_encrypt_AES(v_U32_t cryptHandle, /* Handle */
                           v_U8_t *pPlainText, /* pointer to data stream */
                           v_U8_t *pCiphertext,
                           v_U8_t *pKey) /* pointer to authentication key */
{
   VOS_STATUS uResult = VOS_STATUS_E_FAILURE;
   int retVal = 0;
   AES_KEY aesKey;
   v_U32_t keyLen = AES_BLOCK_SIZE;

    retVal = AES_set_encrypt_key(pKey, keyLen*8, &aesKey);
    if (retVal != 0) {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                   "AES_set_encrypt_key returned %d", retVal);
        return VOS_STATUS_E_FAILURE;
    }
    else {
        uResult = VOS_STATUS_SUCCESS;
    }	
	    

    AES_encrypt(pPlainText, pCiphertext, &aesKey);
#if 0
   BOOL status;
   v_UINT_t uCode = 0;
   v_U8_t  iv[ IV_SIZE_AES_128 ];

   size_t nBuffer = 0;
   v_U8_t *pBuffer = NULL;
   v_U32_t keyLen = KEY_SIZE_AES_128; /* length of authentication key */
   v_U32_t nPlainText = AES_BLOCK_SIZE;
   v_U32_t nCipherText = AES_BLOCK_SIZE;

   DWORD cbEncrypted = ( DWORD ) nPlainText, cbData = ( DWORD ) nCipherText;

   PlainTextKey *pBlob = NULL;

   HCRYPTKEY hKey;
   HCRYPTHASH hHash;
   HCRYPTPROV hCryptProv = (HCRYPTPROV) cryptHandle;
   HMAC_INFO info;

   __try
   {
       nBuffer = sizeof( PlainTextKey ) + keyLen;
       pBuffer = vos_mem_malloc( nBuffer );

       pBlob = ( PlainTextKey* ) pBuffer;
       pBlob->hdr.bType    = PLAINTEXTKEYBLOB;
       pBlob->hdr.bVersion = CUR_BLOB_VERSION;
       pBlob->hdr.reserved = 0;
       pBlob->hdr.aiKeyAlg = CALG_AES_128;
       pBlob->cbKeySize    = 16;
       vos_mem_copy( pBlob + 1, pKey, keyLen );

       status = CryptImportKey( cryptHandle, pBuffer, nBuffer, 0,
                                  CRYPT_IPSEC_HMAC_KEY, &hKey );

       vos_mem_set(iv, IV_SIZE_AES_128, 0);
       status = CryptSetKeyParam( hKey, KP_IV, iv, 0 );

       if (status == 0)
	   {
		   uCode = GetLastError();
           VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
			        " uCode = %d", uCode );
	   }

       vos_mem_copy( pCiphertext, pPlainText, nPlainText );

       status = CryptEncrypt( hKey, 0, TRUE, 0, pCiphertext,
                             &cbEncrypted, cbData );

	   if (status == 0)
	   {
		   uCode = GetLastError();
           VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
               "CryptEncrypt :  pCiphertext = %x cbEncrypted= %d cbData= %d, uCode = %d",
               pCiphertext, cbEncrypted, cbData, uCode );
       }
	   uResult = VOS_STATUS_SUCCESS;
   }

   __except( uCode = GetExceptionCode() )
   {
      if ( STATUS_NO_MEMORY == uCode )	
      {
         uResult = VOS_STATUS_E_NOMEM;
      }
   }
#endif
   return ( uResult );
}

/*--------------------------------------------------------------------------
  
  \brief vos_decrypt_AES() - Decrypts an AES Encrypted byte stream
  
  The vos_decrypt_AES() function decrypts the encrypted byte stream.

  Buffer should be allocated before calling vos_rand_get_bytes().
  
  Attempting to initialize an already initialized lock results in 
  a failure.
 
  \param lock - pointer to the opaque lock object to initialize
  
  \return VOS_STATUS_SUCCESS - Successfully generated random memory.

          VOS_STATUS_E_FAULT  - pbBuf is an invalid pointer.   

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons

       ***VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable
  \sa
   
    ( *** return value not considered yet )
  --------------------------------------------------------------------------*/

VOS_STATUS vos_decrypt_AES(v_U32_t cryptHandle, /* Handle */
                           v_U8_t *pText, /* pointer to data stream */
                           v_U8_t *pDecrypted,
                           v_U8_t *pKey) /* pointer to authentication key */
{
   VOS_STATUS uResult = VOS_STATUS_E_FAILURE;
#if 0
   BOOL status;
   v_UINT_t uCode;

   size_t nBuffer = 0; // Buffer Size
   size_t minSize = 0;
   v_U32_t keyLen = KEY_SIZE_AES_128; /* length of authentication key */
   v_U8_t *pBuffer = NULL;

   PlainTextKey *pBlob = NULL;

   HCRYPTKEY hKey;
   HCRYPTHASH hHash;
   HCRYPTPROV hCryptProv = (HCRYPTPROV) cryptHandle;
   HMAC_INFO info;
//   DWORD cbData = ( DWORD ) textLen;
   DWORD cbMac;

   __try
   {
       nBuffer = sizeof( PlainTextKey ) + KEY_SIZE_AES_128;
       minSize = ( textLen / 16 + 1 ) * 15 + textLen;
       if (nCiphertext < minsize)
       {
           // Error : Insufficient memory
       }
       pBlob = ( PlainTextKey* ) pBuffer;
       pBlob->bh.bType    = PLAINTEXTKEYBLOB;
       pBlob->bh.bVersion = CUR_BLOB_VERSION;
       pBlob->bh.reserved = 0;
       pBlob->bh.aiKeyAlg = CALG_AES_128;
       pBlob->dwKeyLen    = KEY_SIZE_AES_128;
       memcpy( pBlob + 1, pKey, KEY_SIZE_AES_128 );

       status = CryptImportKey( cryptHandle, pBuffer, nBuffer, 0,
                                  CRYPT_IPSEC_HMAC_KEY, hKey.get( ) );

       status = CryptSetKeyParam( hKey, KP_IV, 0, 0 );

       // ASSERT( nCiphertext >= nPlaintext );
       memcpy( pDecrypted, pText, textLen );

       status = CryptDecrypt( hKey, 0, TRUE, 0, pDecrypted, &cbData );
   }
   __except( uCode = GetExceptionCode() )
   {
      if ( STATUS_NO_MEMORY == uCode )	
      {
         uResult = VOS_STATUS_E_NOMEM;
      }
   }
#endif

   return ( uResult );
}
