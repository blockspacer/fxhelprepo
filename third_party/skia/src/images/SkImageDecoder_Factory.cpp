
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkMovie.h"
#include "SkStream.h"
#include "SkTRegistry.h"

typedef SkTRegistry<SkImageDecoder*, SkStream*> DecodeReg;

// N.B. You can't use "DecodeReg::gHead here" due to complex C++
// corner cases.
template DecodeReg* SkTRegistry<SkImageDecoder*, SkStream*>::gHead;

#ifdef SK_ENABLE_LIBBMP
    extern SkImageDecoder* sk_libbmp_dfactory(SkStream* stream);
    static SkTRegistry<SkImageDecoder*, SkStream*> dreg_bmp(sk_libbmp_dfactory);
#endif

#ifdef SK_ENABLE_LIBGIF
    extern SkImageDecoder* sk_libgif_dfactory(SkStream* stream);
    static SkTRegistry<SkImageDecoder*, SkStream*> dreg_gif(sk_libgif_dfactory);
#endif

#if SK_ENABLE_LIBICO
    extern SkImageDecoder* sk_libico_dfactory(SkStream* stream);
    static SkTRegistry<SkImageDecoder*, SkStream*> dreg_ico(sk_libico_dfactory);
#endif

#ifdef SK_ENABLE_LIBJPEG
    extern SkImageDecoder* sk_libjpeg_dfactory(SkStream* stream);
    static SkTRegistry<SkImageDecoder*, SkStream*> dreg_jpg(sk_libjpeg_dfactory);
#endif

#ifdef SK_ENABLE_LIBPNG
    extern SkImageDecoder* sk_libpng_dfactory(SkStream*);
    static SkTRegistry<SkImageDecoder*, SkStream*> dreg_png(sk_libpng_dfactory);
#endif

#ifdef SK_ENABLE_LIBWBMP
    extern SkImageDecoder* sk_wbmp_dfactory(SkStream*);
    static SkTRegistry<SkImageDecoder*, SkStream*> dreg_wbmp(sk_wbmp_dfactory);
#endif

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    SkImageDecoder* codec = NULL;
    const DecodeReg* curr = DecodeReg::Head();
    while (curr) {
        codec = curr->factory()(stream);
        // we rewind here, because we promise later when we call "decode", that
        // the stream will be at its beginning.
        stream->rewind();
        if (codec) {
            return codec;
        }
        curr = curr->next();
    }
#ifdef SK_ENABLE_LIBPNG
    codec = sk_libpng_dfactory(stream);
    stream->rewind();
    if (codec) {
        return codec;
    }
#endif
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

typedef SkTRegistry<SkMovie*, SkStream*> MovieReg;

SkMovie* SkMovie::DecodeStream(SkStream* stream) {
    const MovieReg* curr = MovieReg::Head();
    while (curr) {
        SkMovie* movie = curr->factory()(stream);
        if (movie) {
            return movie;
        }
        // we must rewind only if we got NULL, since we gave the stream to the
        // movie, who may have already started reading from it
        stream->rewind();
        curr = curr->next();
    }
    return NULL;
}