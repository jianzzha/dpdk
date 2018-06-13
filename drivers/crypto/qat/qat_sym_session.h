/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */
#ifndef _QAT_SYM_SESSION_H_
#define _QAT_SYM_SESSION_H_

#include <rte_crypto.h>
#include <rte_cryptodev_pmd.h>

#include "qat_common.h"
#include "icp_qat_hw.h"
#include "icp_qat_fw.h"
#include "icp_qat_fw_la.h"

/*
 * Key Modifier (KM) value used in KASUMI algorithm in F9 mode to XOR
 * Integrity Key (IK)
 */
#define KASUMI_F9_KEY_MODIFIER_4_BYTES   0xAAAAAAAA

#define KASUMI_F8_KEY_MODIFIER_4_BYTES   0x55555555

/* 3DES key sizes */
#define QAT_3DES_KEY_SZ_OPT1 24 /* Keys are independent */
#define QAT_3DES_KEY_SZ_OPT2 16 /* K3=K1 */

#define QAT_AES_HW_CONFIG_CBC_ENC(alg) \
	ICP_QAT_HW_CIPHER_CONFIG_BUILD(ICP_QAT_HW_CIPHER_CBC_MODE, alg, \
					ICP_QAT_HW_CIPHER_NO_CONVERT, \
					ICP_QAT_HW_CIPHER_ENCRYPT)

#define QAT_AES_HW_CONFIG_CBC_DEC(alg) \
	ICP_QAT_HW_CIPHER_CONFIG_BUILD(ICP_QAT_HW_CIPHER_CBC_MODE, alg, \
					ICP_QAT_HW_CIPHER_KEY_CONVERT, \
					ICP_QAT_HW_CIPHER_DECRYPT)

enum qat_crypto_proto_flag {
	QAT_CRYPTO_PROTO_FLAG_NONE = 0,
	QAT_CRYPTO_PROTO_FLAG_CCM = 1,
	QAT_CRYPTO_PROTO_FLAG_GCM = 2,
	QAT_CRYPTO_PROTO_FLAG_SNOW3G = 3,
	QAT_CRYPTO_PROTO_FLAG_ZUC = 4
};

/* Common content descriptor */
struct qat_alg_cd {
	struct icp_qat_hw_cipher_algo_blk cipher;
	struct icp_qat_hw_auth_algo_blk hash;
} __rte_packed __rte_cache_aligned;

struct qat_session {
	enum icp_qat_fw_la_cmd_id qat_cmd;
	enum icp_qat_hw_cipher_algo qat_cipher_alg;
	enum icp_qat_hw_cipher_dir qat_dir;
	enum icp_qat_hw_cipher_mode qat_mode;
	enum icp_qat_hw_auth_algo qat_hash_alg;
	enum icp_qat_hw_auth_op auth_op;
	void *bpi_ctx;
	struct qat_alg_cd cd;
	uint8_t *cd_cur_ptr;
	phys_addr_t cd_paddr;
	struct icp_qat_fw_la_bulk_req fw_req;
	uint8_t aad_len;
	struct qat_crypto_instance *inst;
	struct {
		uint16_t offset;
		uint16_t length;
	} cipher_iv;
	struct {
		uint16_t offset;
		uint16_t length;
	} auth_iv;
	uint16_t digest_length;
	rte_spinlock_t lock;	/* protects this struct */
	enum qat_device_gen min_qat_dev_gen;
};

int
qat_crypto_sym_configure_session(struct rte_cryptodev *dev,
		struct rte_crypto_sym_xform *xform,
		struct rte_cryptodev_sym_session *sess,
		struct rte_mempool *mempool);

int
qat_crypto_set_session_parameters(struct rte_cryptodev *dev,
		struct rte_crypto_sym_xform *xform, void *session_private);

int
qat_crypto_sym_configure_session_aead(struct rte_crypto_sym_xform *xform,
				struct qat_session *session);

int
qat_crypto_sym_configure_session_cipher(struct rte_cryptodev *dev,
		struct rte_crypto_sym_xform *xform,
		struct qat_session *session);

int
qat_crypto_sym_configure_session_auth(struct rte_cryptodev *dev,
				struct rte_crypto_sym_xform *xform,
				struct qat_session *session);

int
qat_alg_aead_session_create_content_desc_cipher(struct qat_session *cd,
						uint8_t *enckey,
						uint32_t enckeylen);

int
qat_alg_aead_session_create_content_desc_auth(struct qat_session *cdesc,
						uint8_t *authkey,
						uint32_t authkeylen,
						uint32_t aad_length,
						uint32_t digestsize,
						unsigned int operation);

int
qat_pmd_session_mempool_create(struct rte_cryptodev *dev,
	unsigned int nb_objs, unsigned int obj_cache_size, int socket_id);

void
qat_crypto_sym_clear_session(struct rte_cryptodev *dev,
		struct rte_cryptodev_sym_session *session);

unsigned int
qat_crypto_sym_get_session_private_size(struct rte_cryptodev *dev);

int qat_get_inter_state_size(enum icp_qat_hw_auth_algo qat_hash_alg);


void qat_alg_init_common_hdr(struct icp_qat_fw_comn_req_hdr *header,
					enum qat_crypto_proto_flag proto_flags);

int qat_alg_validate_aes_key(int key_len, enum icp_qat_hw_cipher_algo *alg);
int qat_alg_validate_aes_docsisbpi_key(int key_len,
					enum icp_qat_hw_cipher_algo *alg);
int qat_alg_validate_snow3g_key(int key_len, enum icp_qat_hw_cipher_algo *alg);
int qat_alg_validate_kasumi_key(int key_len, enum icp_qat_hw_cipher_algo *alg);
int qat_alg_validate_3des_key(int key_len, enum icp_qat_hw_cipher_algo *alg);
int qat_alg_validate_des_key(int key_len, enum icp_qat_hw_cipher_algo *alg);
int qat_cipher_get_block_size(enum icp_qat_hw_cipher_algo qat_cipher_alg);
int qat_alg_validate_zuc_key(int key_len, enum icp_qat_hw_cipher_algo *alg);

#endif /* _QAT_SYM_SESSION_H_ */