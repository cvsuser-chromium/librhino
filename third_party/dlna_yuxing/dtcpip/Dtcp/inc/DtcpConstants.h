//#############################################################################
//## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
//## 
//## The source code contained or described herein and all documents related to
//## the source code ("Material") are owned by Intel Corporation or its 
//## suppliers or licensors. Title to the Material remains with Intel 
//## Corporation or its suppliers and licensors. The Material contains trade 
//## secrets and proprietary and confidential information of Intel or its
//## suppliers and licensors. The Material is protected by worldwide copyright
//## and trade secret laws and treaty provisions. No part of the Material may 
//## be used, copied, reproduced, modified, published, uploaded, posted, 
//## transmitted, distributed, or disclosed in any way without Intel's prior 
//## express written permission.
//## 
//## No license under any patent, copyright, trade secret or other 
//## intellectual property right is granted to or conferred upon you by 
//## disclosure or delivery of the Materials, either expressly, by 
//## implication, inducement, estoppel or otherwise. Any license under such 
//## intellectual property rights must be express and approved by Intel in 
//## writing.
//#############################################################################

#ifndef __DTCP_CONSTANTS_H__
#define __DTCP_CONSTANTS_H__

/// \file
/// \brief Defines constant values used throughout the DTCP-IP SDK

#define DTCP_DEVICE_KEY_COUNT                  12   ///< Number of device keys in the device key matrix

#define DTCP_DEVICE_KEY_SIZE_BITS              64   ///< Number of bits in a device key (device key matrix)
#define DTCP_BASELINE_FULL_CERT_SIZE_BITS     704   ///< Number of bits in a baseline full auth certificate
#define DTCP_EXTENDED_FULL_CERT_SIZE_BITS    1056   ///< Number of bits in a extended auth certificate
#define DTCP_RESTRICTED_CERT_SIZE_BITS        384   ///< Number of bits in a restricted auth certificate
#define DTCP_DEVICE_ID_SIZE_BITS               40   ///< Number of bits in a device ID
#define PUBLIC_KEY_SIZE_BITS                  320   ///< Number of bits in a public key
#define PRIVATE_KEY_SIZE_BITS                 160   ///< Number of bits in a private key
#define SIGNATURE_SIZE_BITS                   320   ///< Number of bits in a digital signature

#define DTCP_DEVICE_KEY_SIZE          (DTCP_DEVICE_KEY_SIZE_BITS / 8)           ///< Size of a device key (device key matrix)
#define DTCP_BASELINE_FULL_CERT_SIZE  (DTCP_BASELINE_FULL_CERT_SIZE_BITS / 8)   ///< Size of a baseline full auth certificate
#define DTCP_EXTENDED_FULL_CERT_SIZE  (DTCP_EXTENDED_FULL_CERT_SIZE_BITS / 8)   ///< Size of a extended auth certificate
#define DTCP_RESTRICTED_CERT_SIZE     (DTCP_RESTRICTED_CERT_SIZE_BITS / 8)      ///< Size of a restricted auth certificate
#define DTCP_DEVICE_ID_SIZE           (DTCP_DEVICE_ID_SIZE_BITS / 8)            ///< Size of a device ID
#define PUBLIC_KEY_SIZE               (PUBLIC_KEY_SIZE_BITS / 8)                ///< Size of a public key
#define PRIVATE_KEY_SIZE              (PRIVATE_KEY_SIZE_BITS / 8)               ///< Size of a private key
#define SIGNATURE_SIZE                (SIGNATURE_SIZE_BITS / 8)                 ///< Size of a digital signature

// AKE data sizes
#define DTCP_FULL_AUTH_NONCE_SIZE_BITS                  128 ///< Number of bits in the random nonce for full auth
#define DTCP_RESTRICTED_AUTH_NONCE_SIZE_BITS             64 ///< Number of bits in the random nonce for restricted auth
#define DTCP_RESTRICTED_AUTH_RESPONSE_SIZE_BITS          64 ///< Number of bits in the response for restricted auth
#define DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE_BITS          320 ///< Number of bits in the Diffie-Hellman first phase value
#define DTCP_DH_FIRST_PHASE_SECRET_SIZE_BITS            160 ///< Number of bits in the Diffie-Hellman first phase secret
#define DTCP_AUTH_KEY_SIZE_BITS                          96 ///< Number of bits in the authentication key
#define DTCP_RESTRICTED_AUTH_KEY_SIZE_BITS               64 ///< Number of bits in the restrcited auth authentication key
#define DTCP_KEY_SELECTION_VECTOR_SIZE_BITS              12 ///< Number of bits in the key selection vector
#define DTCP_EXCHANGE_KEY_SIZE_BITS                      96 ///< Number of bits in the exchange key
#define DTCP_CONTENT_KEY_NONCE_SIZE_BITS                 64 ///< Number of bits in the nonce for computing the content key
//#define DTCP_BASELINE_KEY_SIZE_BITS                      56 ///< Number of bits in the baseline content key
#define DTCP_SRM_VERSION_NUMBER_SIZE_BITS                16 ///< Number of bits in the SRM version number
#define DTCP_EXCHANGE_KEY_LABEL_SIZE_BITS                 8 ///< Number of bits in the exchange key label
#define DTCP_CONTENT_KEY_CONSTANT_SIZE_BITS              96 ///< Number of bits in the content key constant
#define DTCP_IV_CONSTANT_SIZE_BITS                       64 ///< Number of bits in the content key IV
#define DTCP_IP_CONTENT_KEY_SIZE_BITS                   128 ///< Number of bits in the content key for IP

#define DTCP_FULL_AUTH_NONCE_SIZE                   (DTCP_FULL_AUTH_NONCE_SIZE_BITS / 8)            ///< Size of the random nonce for full auth
#define DTCP_RESTRICTED_AUTH_NONCE_SIZE             (DTCP_RESTRICTED_AUTH_NONCE_SIZE_BITS / 8)      ///< Size of the random nonce for restricted auth
#define DTCP_RESTRICTED_AUTH_RESPONSE_SIZE          (DTCP_RESTRICTED_AUTH_RESPONSE_SIZE_BITS / 8)   ///< Size of the response for restricted auth
#define DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE           (DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE_BITS / 8)    ///< Size of the Diffie-Hellman first phase value
#define DTCP_DH_FIRST_PHASE_SECRET_SIZE             (DTCP_DH_FIRST_PHASE_SECRET_SIZE_BITS / 8)      ///< Size of the Diffie-Hellman first phase secret
#define DTCP_AUTH_KEY_SIZE                          (DTCP_AUTH_KEY_SIZE_BITS / 8)                   ///< Size of the authentication key
#define DTCP_RESTRICTED_AUTH_KEY_SIZE               (DTCP_RESTRICTED_AUTH_KEY_SIZE_BITS / 8)        ///< Size of the restrcited auth authentication key
#define DTCP_EXCHANGE_KEY_SIZE                      (DTCP_EXCHANGE_KEY_SIZE_BITS / 8)               ///< Size of the exchange key
#define DTCP_CONTENT_KEY_NONCE_SIZE                 (DTCP_CONTENT_KEY_NONCE_SIZE_BITS / 8)          ///< Size of the nonce for computing the content key
//#define DTCP_BASELINE_KEY_SIZE                      (DTCP_BASELINE_KEY_SIZE_BITS / 8)             ///< Size of the baseline content key
#define DTCP_SRM_VERSION_NUMBER_SIZE                (DTCP_SRM_VERSION_NUMBER_SIZE_BITS / 8)         ///< Size of the SRM version number
#define DTCP_EXCHANGE_KEY_LABEL_SIZE                (DTCP_EXCHANGE_KEY_LABEL_SIZE_BITS / 8)         ///< Size of the exchange key label
#define DTCP_CONTENT_KEY_CONSTANT_SIZE              (DTCP_CONTENT_KEY_CONSTANT_SIZE_BITS / 8)       ///< Size of the content key constant
#define DTCP_IV_CONSTANT_SIZE                       (DTCP_IV_CONSTANT_SIZE_BITS / 8)                ///< Size of the content key IV
#define DTCP_IP_CONTENT_KEY_SIZE                    (DTCP_IP_CONTENT_KEY_SIZE_BITS / 8)             ///< Size of the content key for IP
                                                                                                    
#define DTCP_FULL_AUTH_CHALLENGE_SIZE_BITS          (DTCP_FULL_AUTH_NONCE_SIZE_BITS + DTCP_BASELINE_FULL_CERT_SIZE_BITS)                ///< Number of bits in a full auth challenge
#define DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE_BITS    (DTCP_RESTRICTED_AUTH_NONCE_SIZE_BITS + DTCP_KEY_SELECTION_VECTOR_SIZE_BITS + 4)    ///< Number of bits in a restricted auth challenge
#define DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE_BITS (DTCP_RESTRICTED_CERT_SIZE_BITS + DTCP_RESTRICTED_AUTH_NONCE_SIZE_BITS)       ///< Number of bits in a enhanced restricted auth challenge
#define DTCP_FULL_AUTH_RESPONSE_SIZE_BITS           (DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE_BITS + DTCP_SRM_VERSION_NUMBER_SIZE_BITS + 8 + SIGNATURE_SIZE_BITS) ///< Number of bits in a full auth response
#define DTCP_EXCHANGE_KEY_CMD_DATA_SIZE_BITS        (8 + 8 + DTCP_EXCHANGE_KEY_SIZE_BITS)                                               ///< Number of bits in a exchange key command
#define DTCP_CONTENT_KEY_REQUEST_SIZE_BITS          (DTCP_EXCHANGE_KEY_LABEL_SIZE_BITS + 4 + 19 + 1 + DTCP_CONTENT_KEY_NONCE_SIZE_BITS) ///< Number of bits in a content key request

#define DTCP_FULL_AUTH_CHALLENGE_SIZE               (DTCP_FULL_AUTH_CHALLENGE_SIZE_BITS / 8)                    ///< Size of a full auth challenge
#define DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE         (DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE_BITS / 8)              ///< Size of a restricted auth challenge
#define DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE (DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE_BITS / 8)    ///< Size of a enhanced restricted auth challenge
#define DTCP_FULL_AUTH_RESPONSE_SIZE                (DTCP_FULL_AUTH_RESPONSE_SIZE_BITS / 8)                     ///< Size of a full auth response
#define DTCP_EXCHANGE_KEY_CMD_DATA_SIZE             (DTCP_EXCHANGE_KEY_CMD_DATA_SIZE_BITS / 8)                  ///< Size of a exchange key command
#define DTCP_CONTENT_KEY_REQUEST_SIZE               (DTCP_CONTENT_KEY_REQUEST_SIZE_BITS / 8)                    ///< Size of a content key request

#define DTCP_SRM_HEADER_SIZE                    4   ///< Size of a SRM header
#define DTCP_SRM_CRL_LENGTH_SIZE                2   ///< Size of a SRM CRL length field

#define DTCP_SRM_FIRST_GEN                      0   ///< First generation SRM
#define DTCP_SRM_SECOND_GEN                     1   ///< Second generation SRM

#define DTCP_SRM_FIRST_GEN_MAX_SIZE           128   ///< Maximum size of a first generation SRM
#define DTCP_SRM_SECOND_GEN_MAX_SIZE         1024   ///< Maximum size of a second generation SRM
#define DTCP_SRM_MAX_SIZE                    DTCP_SRM_SECOND_GEN_MAX_SIZE   ///< Maximum SRM size
#define DTCP_SRM_CRL_MIN_SIZE                (DTCP_SRM_CRL_LENGTH_SIZE + SIGNATURE_SIZE) ///< Minimum SRM CRL size

#define DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE    (DTCP_FULL_AUTH_NONCE_SIZE + \
                                                     DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + \
                                                     DTCP_SRM_VERSION_NUMBER_SIZE + \
                                                     1)   ///< Size of the full auth response buffer that is signed

#define DTCP_CONTENT_PACKET_HEADER_SIZE          14 ///< Size of a protected content packet header

#define DTCP_AES_KEY_SIZE_BITS                  128 ///< Number of bits in an AES key
#define DTCP_AES_KEY_SIZE                       (DTCP_AES_KEY_SIZE_BITS / 8)    ///< Size of an AES key

#define DTCP_AES_IV_SIZE_BITS                   128 ///< Number of bits in an AES IV
#define DTCP_AES_IV_SIZE                        (DTCP_AES_IV_SIZE_BITS / 8)     ///< Size of an AES IV

#define DTCP_AES_BLOCK_SIZE_BITS                128 ///< Number of bits in an AES block
#define DTCP_AES_BLOCK_SIZE                     (DTCP_AES_BLOCK_SIZE_BITS / 8)  ///< Size of an AES block

#define DTCP_MAXIMUM_PROTECTED_PACKET_SIZE      134217728   ///< Maximum payload size of a protected content packet

#define DTCP_SINK_COUNT_LIMIT                    32 ///< Sink count limit

#define DTCP_RTP_NONCE_TIMER_PERIOD              60 ///< 60 seconds

// These should be in ECC.h
#define ECC_PRIME_NUMBER_SIZE_BITS              160 ///< Number of bits in the ECC prime number
#define ECC_COEFFICIENT_SIZE_BITS               160 ///< Number of bits in the ECC coefficient
#define ECC_BASEPOINT_SIZE_BITS                 320 ///< Number of bits in the ECC basepoint
#define ECC_BASEPOINT_ORDER_SIZE_BITS           160 ///< Number of bits in the ECC basepoint order
#define ECC_PUBLIC_KEY_SIZE_BITS                320 ///< Number of bits in the ECC public key
#define ECC_PRIVATE_KEY_SIZE_BITS               160 ///< Number of bits in the ECC private key

#define ECC_PRIME_NUMBER_SIZE           (ECC_PRIME_NUMBER_SIZE_BITS / 8)    ///< Size of the ECC prime number
#define ECC_COEFFICIENT_SIZE            (ECC_COEFFICIENT_SIZE_BITS / 8)     ///< Size of the ECC coefficient
#define ECC_BASEPOINT_SIZE              (ECC_BASEPOINT_SIZE_BITS / 8)       ///< Size of the ECC basepoint 
#define ECC_BASEPOINT_ORDER_SIZE        (ECC_BASEPOINT_ORDER_SIZE_BITS / 8) ///< Size of the ECC basepoint order
#define ECC_PUBLIC_KEY_SIZE             (ECC_PUBLIC_KEY_SIZE_BITS / 8)      ///< Size of the ECC public key
#define ECC_PRIVATE_KEY_SIZE            (ECC_PRIVATE_KEY_SIZE_BITS / 8)     ///< Size of the ECC private key


#define TRUE  1 ///< True
#define FALSE 0 ///< False

#endif // __DTCP_CONSTANTS_H__
