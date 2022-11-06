LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_ROOT := $(LOCAL_PATH)/faad2/libfaad

LOCAL_MODULE    := faad2
LOCAL_SRC_FILES := ${SRC_ROOT}/bits.c \
					${SRC_ROOT}/cfft.c \
					${SRC_ROOT}/common.c \
					${SRC_ROOT}/decoder.c \
					${SRC_ROOT}/drc.c \
					${SRC_ROOT}/drm_dec.c \
					${SRC_ROOT}/error.c \
					${SRC_ROOT}/filtbank.c \
					${SRC_ROOT}/hcr.c \
					${SRC_ROOT}/huffman.c \
					${SRC_ROOT}/ic_predict.c \
					${SRC_ROOT}/is.c \
					${SRC_ROOT}/lt_predict.c \
					${SRC_ROOT}/mdct.c \
					${SRC_ROOT}/mp4.c \
					${SRC_ROOT}/ms.c \
					${SRC_ROOT}/output.c \
					${SRC_ROOT}/pns.c \
					${SRC_ROOT}/ps_dec.c \
					${SRC_ROOT}/ps_syntax.c \
					${SRC_ROOT}/pulse.c \
					${SRC_ROOT}/rvlc.c \
					${SRC_ROOT}/sbr_dct.c \
					${SRC_ROOT}/sbr_dec.c \
					${SRC_ROOT}/sbr_e_nf.c \
					${SRC_ROOT}/sbr_fbt.c \
					${SRC_ROOT}/sbr_hfadj.c \
					${SRC_ROOT}/sbr_hfgen.c \
					${SRC_ROOT}/sbr_huff.c \
					${SRC_ROOT}/sbr_qmf.c \
					${SRC_ROOT}/sbr_syntax.c \
					${SRC_ROOT}/sbr_tf_grid.c \
					${SRC_ROOT}/specrec.c \
					${SRC_ROOT}/ssr_fb.c \
					${SRC_ROOT}/ssr_ipqf.c \
					${SRC_ROOT}/ssr.c \
					${SRC_ROOT}/syntax.c \
					${SRC_ROOT}/tns.c 

LOCAL_C_INCLUDES := $(LOCAL_PATH)/faad2/include $(LOCAL_PATH)/faad2/libfaad

LOCAL_CFLAGS := -DHAVE_CONFIG_H 

include $(BUILD_STATIC_LIBRARY)
