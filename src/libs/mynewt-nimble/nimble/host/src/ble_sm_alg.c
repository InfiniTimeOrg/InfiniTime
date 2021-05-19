/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* These functions are adapted from the Intel Zephyr BLE security manager
 * code.
 */

#include <inttypes.h>
#include <string.h>
#include "syscfg/syscfg.h"
#include "nimble/nimble_opt.h"

#if NIMBLE_BLE_CONNECT
#if NIMBLE_BLE_SM

#include "nimble/ble.h"
#include "ble_hs_priv.h"
#include "tinycrypt/aes.h"
#include "tinycrypt/constants.h"
#include "tinycrypt/utils.h"

#if MYNEWT_VAL(BLE_SM_SC)
#include "tinycrypt/cmac_mode.h"
#include "tinycrypt/ecc_dh.h"
#if MYNEWT_VAL(TRNG)
#include "trng/trng.h"
#endif
#endif

#if MYNEWT_VAL(BLE_SM_SC) && MYNEWT_VAL(TRNG)
static struct trng_dev *g_trng;
#endif

static void
ble_sm_alg_xor_128(const uint8_t *p, const uint8_t *q, uint8_t *r)
{
    int i;

    for (i = 0; i < 16; i++) {
        r[i] = p[i] ^ q[i];
    }
}

static int
ble_sm_alg_encrypt(const uint8_t *key, const uint8_t *plaintext,
                   uint8_t *enc_data)
{
    struct tc_aes_key_sched_struct s;
    uint8_t tmp[16];

    swap_buf(tmp, key, 16);

    if (tc_aes128_set_encrypt_key(&s, tmp) == TC_CRYPTO_FAIL) {
        return BLE_HS_EUNKNOWN;
    }

    swap_buf(tmp, plaintext, 16);

    if (tc_aes_encrypt(enc_data, tmp, &s) == TC_CRYPTO_FAIL) {
        return BLE_HS_EUNKNOWN;
    }

    swap_in_place(enc_data, 16);

    return 0;
}

int
ble_sm_alg_s1(const uint8_t *k, const uint8_t *r1, const uint8_t *r2,
              uint8_t *out)
{
    int rc;

    /* The most significant 64-bits of r1 are discarded to generate
     * r1' and the most significant 64-bits of r2 are discarded to
     * generate r2'.
     * r1' is concatenated with r2' to generate r' which is used as
     * the 128-bit input parameter plaintextData to security function e:
     *
     *    r' = r1' || r2'
     */
    memcpy(out, r2, 8);
    memcpy(out + 8, r1, 8);

    /* s1(k, r1 , r2) = e(k, r') */
    rc = ble_sm_alg_encrypt(k, out, out);
    if (rc != 0) {
        return rc;
    }

    BLE_HS_LOG(DEBUG, "ble_sm_alg_s1()\n    k=");
    ble_hs_log_flat_buf(k, 16);
    BLE_HS_LOG(DEBUG, "\n    r1=");
    ble_hs_log_flat_buf(r1, 16);
    BLE_HS_LOG(DEBUG, "\n    r2=");
    ble_hs_log_flat_buf(r2, 16);
    BLE_HS_LOG(DEBUG, "\n    out=");
    ble_hs_log_flat_buf(out, 16);
    BLE_HS_LOG(DEBUG, "\n");

    return 0;
}

int
ble_sm_alg_c1(const uint8_t *k, const uint8_t *r,
              const uint8_t *preq, const uint8_t *pres,
              uint8_t iat, uint8_t rat,
              const uint8_t *ia, const uint8_t *ra,
              uint8_t *out_enc_data)
{
    uint8_t p1[16], p2[16];
    int rc;

    BLE_HS_LOG(DEBUG, "ble_sm_alg_c1()\n    k=");
    ble_hs_log_flat_buf(k, 16);
    BLE_HS_LOG(DEBUG, "\n    r=");
    ble_hs_log_flat_buf(r, 16);
    BLE_HS_LOG(DEBUG, "\n    iat=%d rat=%d", iat, rat);
    BLE_HS_LOG(DEBUG, "\n    ia=");
    ble_hs_log_flat_buf(ia, 6);
    BLE_HS_LOG(DEBUG, "\n    ra=");
    ble_hs_log_flat_buf(ra, 6);
    BLE_HS_LOG(DEBUG, "\n    preq=");
    ble_hs_log_flat_buf(preq, 7);
    BLE_HS_LOG(DEBUG, "\n    pres=");
    ble_hs_log_flat_buf(pres, 7);

    /* pres, preq, rat and iat are concatenated to generate p1 */
    p1[0] = iat;
    p1[1] = rat;
    memcpy(p1 + 2, preq, 7);
    memcpy(p1 + 9, pres, 7);

    BLE_HS_LOG(DEBUG, "\n    p1=");
    ble_hs_log_flat_buf(p1, sizeof p1);

    /* c1 = e(k, e(k, r XOR p1) XOR p2) */

    /* Using out_enc_data as temporary output buffer */
    ble_sm_alg_xor_128(r, p1, out_enc_data);

    rc = ble_sm_alg_encrypt(k, out_enc_data, out_enc_data);
    if (rc != 0) {
        rc = BLE_HS_EUNKNOWN;
        goto done;
    }

    /* ra is concatenated with ia and padding to generate p2 */
    memcpy(p2, ra, 6);
    memcpy(p2 + 6, ia, 6);
    memset(p2 + 12, 0, 4);

    BLE_HS_LOG(DEBUG, "\n    p2=");
    ble_hs_log_flat_buf(p2, sizeof p2);

    ble_sm_alg_xor_128(out_enc_data, p2, out_enc_data);

    rc = ble_sm_alg_encrypt(k, out_enc_data, out_enc_data);
    if (rc != 0) {
        rc = BLE_HS_EUNKNOWN;
        goto done;
    }

    BLE_HS_LOG(DEBUG, "\n    out_enc_data=");
    ble_hs_log_flat_buf(out_enc_data, 16);

    rc = 0;

done:
    BLE_HS_LOG(DEBUG, "\n    rc=%d\n", rc);
    return rc;
}

#if MYNEWT_VAL(BLE_SM_SC)

static void
ble_sm_alg_log_buf(const char *name, const uint8_t *buf, int len)
{
    BLE_HS_LOG(DEBUG, "    %s=", name);
    ble_hs_log_flat_buf(buf, len);
    BLE_HS_LOG(DEBUG, "\n");
}

/**
 * Cypher based Message Authentication Code (CMAC) with AES 128 bit
 *
 * @param key                   128-bit key.
 * @param in                    Message to be authenticated.
 * @param len                   Length of the message in octets.
 * @param out                   Output; message authentication code.
 */
static int
ble_sm_alg_aes_cmac(const uint8_t *key, const uint8_t *in, size_t len,
                    uint8_t *out)
{
    struct tc_aes_key_sched_struct sched;
    struct tc_cmac_struct state;

    if (tc_cmac_setup(&state, key, &sched) == TC_CRYPTO_FAIL) {
        return BLE_HS_EUNKNOWN;
    }

    if (tc_cmac_update(&state, in, len) == TC_CRYPTO_FAIL) {
        return BLE_HS_EUNKNOWN;
    }

    if (tc_cmac_final(out, &state) == TC_CRYPTO_FAIL) {
        return BLE_HS_EUNKNOWN;
    }

    return 0;
}

int
ble_sm_alg_f4(const uint8_t *u, const uint8_t *v, const uint8_t *x,
              uint8_t z, uint8_t *out_enc_data)
{
    uint8_t xs[16];
    uint8_t m[65];
    int rc;

    BLE_HS_LOG(DEBUG, "ble_sm_alg_f4()\n    u=");
    ble_hs_log_flat_buf(u, 32);
    BLE_HS_LOG(DEBUG, "\n    v=");
    ble_hs_log_flat_buf(v, 32);
    BLE_HS_LOG(DEBUG, "\n    x=");
    ble_hs_log_flat_buf(x, 16);
    BLE_HS_LOG(DEBUG, "\n    z=0x%02x\n", z);

    /*
     * U, V and Z are concatenated and used as input m to the function
     * AES-CMAC and X is used as the key k.
     *
     * Core Spec 4.2 Vol 3 Part H 2.2.5
     *
     * note:
     * ble_sm_alg_aes_cmac uses BE data; ble_sm_alg_f4 accepts LE so we swap.
     */
    swap_buf(m, u, 32);
    swap_buf(m + 32, v, 32);
    m[64] = z;

    swap_buf(xs, x, 16);

    rc = ble_sm_alg_aes_cmac(xs, m, sizeof(m), out_enc_data);
    if (rc != 0) {
        return BLE_HS_EUNKNOWN;
    }

    swap_in_place(out_enc_data, 16);

    BLE_HS_LOG(DEBUG, "    out_enc_data=");
    ble_hs_log_flat_buf(out_enc_data, 16);
    BLE_HS_LOG(DEBUG, "\n");

    return 0;
}

int
ble_sm_alg_f5(const uint8_t *w, const uint8_t *n1, const uint8_t *n2,
              uint8_t a1t, const uint8_t *a1, uint8_t a2t, const uint8_t *a2,
              uint8_t *mackey, uint8_t *ltk)
{
    static const uint8_t salt[16] = { 0x6c, 0x88, 0x83, 0x91, 0xaa, 0xf5,
                      0xa5, 0x38, 0x60, 0x37, 0x0b, 0xdb,
                      0x5a, 0x60, 0x83, 0xbe };
    uint8_t m[53] = {
        0x00, /* counter */
        0x62, 0x74, 0x6c, 0x65, /* keyID */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*n1*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*2*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* a1 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* a2 */
        0x01, 0x00 /* length */
    };
    uint8_t ws[32];
    uint8_t t[16];
    int rc;

    BLE_HS_LOG(DEBUG, "ble_sm_alg_f5()\n");
    ble_sm_alg_log_buf("w", w, 32);
    ble_sm_alg_log_buf("n1", n1, 16);
    ble_sm_alg_log_buf("n2", n2, 16);

    swap_buf(ws, w, 32);

    rc = ble_sm_alg_aes_cmac(salt, ws, 32, t);
    if (rc != 0) {
        return BLE_HS_EUNKNOWN;
    }

    ble_sm_alg_log_buf("t", t, 16);

    swap_buf(m + 5, n1, 16);
    swap_buf(m + 21, n2, 16);
    m[37] = a1t;
    swap_buf(m + 38, a1, 6);
    m[44] = a2t;
    swap_buf(m + 45, a2, 6);

    rc = ble_sm_alg_aes_cmac(t, m, sizeof(m), mackey);
    if (rc != 0) {
        return BLE_HS_EUNKNOWN;
    }

    ble_sm_alg_log_buf("mackey", mackey, 16);

    swap_in_place(mackey, 16);

    /* Counter for ltk is 1. */
    m[0] = 0x01;

    rc = ble_sm_alg_aes_cmac(t, m, sizeof(m), ltk);
    if (rc != 0) {
        return BLE_HS_EUNKNOWN;
    }

    ble_sm_alg_log_buf("ltk", ltk, 16);

    swap_in_place(ltk, 16);

    return 0;
}

int
ble_sm_alg_f6(const uint8_t *w, const uint8_t *n1, const uint8_t *n2,
              const uint8_t *r, const uint8_t *iocap, uint8_t a1t,
              const uint8_t *a1, uint8_t a2t, const uint8_t *a2,
              uint8_t *check)
{
    uint8_t ws[16];
    uint8_t m[65];
    int rc;

    BLE_HS_LOG(DEBUG, "ble_sm_alg_f6()\n");
    ble_sm_alg_log_buf("w", w, 16);
    ble_sm_alg_log_buf("n1", n1, 16);
    ble_sm_alg_log_buf("n2", n2, 16);
    ble_sm_alg_log_buf("r", r, 16);
    ble_sm_alg_log_buf("iocap", iocap, 3);
    ble_sm_alg_log_buf("a1t", &a1t, 1);
    ble_sm_alg_log_buf("a1", a1, 6);
    ble_sm_alg_log_buf("a2t", &a2t, 1);
    ble_sm_alg_log_buf("a2", a2, 6);

    swap_buf(m, n1, 16);
    swap_buf(m + 16, n2, 16);
    swap_buf(m + 32, r, 16);
    swap_buf(m + 48, iocap, 3);

    m[51] = a1t;
    memcpy(m + 52, a1, 6);
    swap_buf(m + 52, a1, 6);

    m[58] = a2t;
    memcpy(m + 59, a2, 6);
    swap_buf(m + 59, a2, 6);

    swap_buf(ws, w, 16);

    rc = ble_sm_alg_aes_cmac(ws, m, sizeof(m), check);
    if (rc != 0) {
        return BLE_HS_EUNKNOWN;
    }

    ble_sm_alg_log_buf("res", check, 16);

    swap_in_place(check, 16);

    return 0;
}

int
ble_sm_alg_g2(const uint8_t *u, const uint8_t *v, const uint8_t *x,
              const uint8_t *y, uint32_t *passkey)
{
    uint8_t m[80], xs[16];
    int rc;

    BLE_HS_LOG(DEBUG, "ble_sm_alg_g2()\n");
    ble_sm_alg_log_buf("u", u, 32);
    ble_sm_alg_log_buf("v", v, 32);
    ble_sm_alg_log_buf("x", x, 16);
    ble_sm_alg_log_buf("y", y, 16);

    swap_buf(m, u, 32);
    swap_buf(m + 32, v, 32);
    swap_buf(m + 64, y, 16);

    swap_buf(xs, x, 16);

    /* reuse xs (key) as buffer for result */
    rc = ble_sm_alg_aes_cmac(xs, m, sizeof(m), xs);
    if (rc != 0) {
        return BLE_HS_EUNKNOWN;
    }

    ble_sm_alg_log_buf("res", xs, 16);

    *passkey = get_be32(xs + 12) % 1000000;
    BLE_HS_LOG(DEBUG, "    passkey=%u\n", *passkey);

    return 0;
}

int
ble_sm_alg_gen_dhkey(const uint8_t *peer_pub_key_x, const uint8_t *peer_pub_key_y,
                     const uint8_t *our_priv_key, uint8_t *out_dhkey)
{
    uint8_t dh[32];
    uint8_t pk[64];
    uint8_t priv[32];
    int rc;

    swap_buf(pk, peer_pub_key_x, 32);
    swap_buf(&pk[32], peer_pub_key_y, 32);
    swap_buf(priv, our_priv_key, 32);

    if (uECC_valid_public_key(pk, &curve_secp256r1) < 0) {
        return BLE_HS_EUNKNOWN;
    }

    rc = uECC_shared_secret(pk, priv, dh, &curve_secp256r1);
    if (rc == TC_CRYPTO_FAIL) {
        return BLE_HS_EUNKNOWN;
    }

    swap_buf(out_dhkey, dh, 32);

    return 0;
}

/* based on Core Specification 4.2 Vol 3. Part H 2.3.5.6.1 */
static const uint8_t ble_sm_alg_dbg_priv_key[32] = {
    0x3f, 0x49, 0xf6, 0xd4, 0xa3, 0xc5, 0x5f, 0x38, 0x74, 0xc9, 0xb3, 0xe3,
    0xd2, 0x10, 0x3f, 0x50, 0x4a, 0xff, 0x60, 0x7b, 0xeb, 0x40, 0xb7, 0x99,
    0x58, 0x99, 0xb8, 0xa6, 0xcd, 0x3c, 0x1a, 0xbd
};

#if MYNEWT_VAL(BLE_SM_SC_DEBUG_KEYS)
static const uint8_t ble_sm_alg_dbg_pub_key[64] = {
    /* X */
    0x20, 0xb0, 0x03, 0xd2, 0xf2, 0x97, 0xbe, 0x2c, 0x5e, 0x2c, 0x83, 0xa7,
    0xe9, 0xf9, 0xa5, 0xb9, 0xef, 0xf4, 0x91, 0x11, 0xac, 0xf4, 0xfd, 0xdb,
    0xcc, 0x03, 0x01, 0x48, 0x0e, 0x35, 0x9d, 0xe6,
    /* Y */
    0xdc, 0x80, 0x9c, 0x49, 0x65, 0x2a, 0xeb, 0x6d, 0x63, 0x32, 0x9a, 0xbf,
    0x5a, 0x52, 0x15, 0x5c, 0x76, 0x63, 0x45, 0xc2, 0x8f, 0xed, 0x30, 0x24,
    0x74, 0x1c, 0x8e, 0xd0, 0x15, 0x89, 0xd2, 0x8b,
};
#endif

/**
 * pub: 64 bytes
 * priv: 32 bytes
 */
int
ble_sm_alg_gen_key_pair(uint8_t *pub, uint8_t *priv)
{
#if MYNEWT_VAL(BLE_SM_SC_DEBUG_KEYS)
    swap_buf(pub, ble_sm_alg_dbg_pub_key, 32);
    swap_buf(&pub[32], &ble_sm_alg_dbg_pub_key[32], 32);
    swap_buf(priv, ble_sm_alg_dbg_priv_key, 32);
#else
    uint8_t pk[64];

    do {
        if (uECC_make_key(pk, priv, &curve_secp256r1) != TC_CRYPTO_SUCCESS) {
            return BLE_HS_EUNKNOWN;
        }

        /* Make sure generated key isn't debug key. */
    } while (memcmp(priv, ble_sm_alg_dbg_priv_key, 32) == 0);

    swap_buf(pub, pk, 32);
    swap_buf(&pub[32], &pk[32], 32);
    swap_in_place(priv, 32);
#endif

    return 0;
}

#if MYNEWT_VAL(SELFTEST)
/* Unit tests rely on custom RNG function not being set */
#define ble_sm_alg_rand NULL
#else
/* used by uECC to get random data */
static int
ble_sm_alg_rand(uint8_t *dst, unsigned int size)
{
#if MYNEWT_VAL(TRNG)
    size_t num;

    if (!g_trng) {
        g_trng = (struct trng_dev *)os_dev_open("trng", OS_WAIT_FOREVER, NULL);
        assert(g_trng);
    }

    while (size) {
        num = trng_read(g_trng, dst, size);
        dst += num;
        size -= num;
    }
#else
    if (ble_hs_hci_util_rand(dst, size)) {
        return 0;
    }
#endif

    return 1;
}
#endif

void
ble_sm_alg_ecc_init(void)
{
    uECC_set_rng(ble_sm_alg_rand);
}

#endif
#endif
#endif
