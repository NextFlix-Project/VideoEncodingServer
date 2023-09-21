#pragma once

#include "../3rdParty/crow/crow.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

namespace NextFlix
{
    class VideoEncoder
    {
    public:
        int encodeVideo(const char *inputFilePath, const char *outputFilePath, const char *codecName);
        int encodeVideoWithCmd(std::string inputFilePath, std::string outputFilePath);
        crow::json::wvalue getAvailableCodecs();

    private:
        int encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile);
    };
};