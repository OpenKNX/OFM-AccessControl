#pragma once
// Existing PN7160 applications opt into the common service automatically.
// Explicit PN7161/ST25 builds already define OPENKNX_ACC_NFC_READER.
#if !defined(OPENKNX_ACC_NFC_READER) && !defined(OPENKNX_ACC_DISABLE_LEGACY_NFC)
#define OPENKNX_ACC_NFC_READER
#define OPENKNX_NFC_PN7160
#endif
