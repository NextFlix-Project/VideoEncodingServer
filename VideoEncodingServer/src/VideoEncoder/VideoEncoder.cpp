#include "VideoEncoder.h"

using namespace NextFlix;

int VideoEncoder::encodeVideoWithCmd(std::string inputFilePath, std::string outputFilePath){
        std::string cmd = "ffmpeg -i " + inputFilePath + " -c:v libx264 -b:v 2m -c:a aac -b:a 128k -f dash " + outputFilePath;

        
        int result = std::system(cmd.c_str());

        return result;
}

int VideoEncoder::encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
    int ret;

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0)
    {
        return 1;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 1;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        if (frame && frame->best_effort_timestamp != AV_NOPTS_VALUE)
        {
            pkt->pts = av_rescale_q(frame->best_effort_timestamp, enc_ctx->time_base, enc_ctx->pkt_timebase);
            pkt->dts = av_rescale_q(frame->pkt_dts, enc_ctx->time_base, enc_ctx->pkt_timebase);
        }
        else
        {
            pkt->pts = av_rescale_q(enc_ctx->frame_number, enc_ctx->time_base, enc_ctx->pkt_timebase);
            pkt->dts = pkt->pts;
        }

        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }

    return 0;
}

int VideoEncoder::encodeVideo(const char *inputFilePath, const char *outputFilePath, const char *codecName)
{
        AVFormatContext *inputFormatContext = NULL;
        AVCodecContext *inputCodecContext = NULL;
        AVCodecContext *outputCodecContext = NULL;
        AVStream *inputVideoStream = NULL;
        AVFrame *inputFrame = NULL;
        AVPacket *outputPacket = NULL;
        SwsContext *swsContext = NULL;

        int videoStreamIndex = -1;
        int ret;

        ret = avformat_open_input(&inputFormatContext, inputFilePath, NULL, NULL);
        if (ret < 0)
        {
            return 1;
        }

        ret = avformat_find_stream_info(inputFormatContext, NULL);
        if (ret < 0)
        {
            return 2;
        }

        for (unsigned int i = 0; i < inputFormatContext->nb_streams; i++)
        {
            if (inputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1)
        {
            return 3;
        }

        inputVideoStream = inputFormatContext->streams[videoStreamIndex];

        const AVCodec *inputCodec = avcodec_find_decoder(inputVideoStream->codecpar->codec_id);

        if (!inputCodec)
        {
            return 4;
        }

        inputCodecContext = avcodec_alloc_context3(inputCodec);

        if (!inputCodecContext)
        {
            return 5;
        }

        ret = avcodec_parameters_to_context(inputCodecContext, inputVideoStream->codecpar);
        if (ret < 0)
        {
            return 6;
        }

        ret = avcodec_open2(inputCodecContext, inputCodec, NULL);
        if (ret < 0)
        {
            return 7;
        }

        inputFrame = av_frame_alloc();
        if (!inputFrame)
        {
            return 8;
        }

        const AVCodec *outputCodec = avcodec_find_encoder_by_name("libvpx-vp9");
        if (!outputCodec)
        {
            return 9;
        }

        outputCodecContext = avcodec_alloc_context3(outputCodec);
        if (!outputCodecContext)
        {
            return 10;
        }

        outputCodecContext->bit_rate = 40000;
        outputCodecContext->width = inputCodecContext->width;
        outputCodecContext->height = inputCodecContext->height;
        outputCodecContext->time_base = (AVRational){1, 25};
        outputCodecContext->framerate = (AVRational){25, 1};
        outputCodecContext->gop_size = 10;
        outputCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

        ret = avcodec_open2(outputCodecContext, outputCodec, NULL);
        if (ret < 0)
        {
            return 11;
        }

        outputPacket = av_packet_alloc();
        if (!outputPacket)
        {
            return 12;
        }

        swsContext = sws_getContext(inputCodecContext->width, inputCodecContext->height, inputCodecContext->pix_fmt,
                                    outputCodecContext->width, outputCodecContext->height, outputCodecContext->pix_fmt,
                                    SWS_BILINEAR, NULL, NULL, NULL);
        if (!swsContext)
        {
            return 13;
        }

        FILE *outputFile = fopen(outputFilePath, "wb");
        if (!outputFile)
        {
            return 14;
        }

        AVFrame *outputFrame = av_frame_alloc();
        if (!outputFrame)
        {
            return 15;
        }

        outputFrame->format = outputCodecContext->pix_fmt;
        outputFrame->width = outputCodecContext->width;
        outputFrame->height = outputCodecContext->height;

        ret = av_frame_get_buffer(outputFrame, 0);
        if (ret < 0)
        {
            return 16;
        }

        int64_t totalFrames = inputFormatContext->streams[videoStreamIndex]->nb_frames;
        int64_t framesProcessed = 0;

        while (av_read_frame(inputFormatContext, outputPacket) >= 0)
        {
            if (outputPacket->stream_index == videoStreamIndex)
            {
                ret = avcodec_send_packet(inputCodecContext, outputPacket);
                if (ret < 0)
                {
                    break;
                }

                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(inputCodecContext, inputFrame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }
                    else if (ret < 0)
                    {
                        return 17;
                    }

                    sws_scale(swsContext, (const uint8_t *const *)inputFrame->data, inputFrame->linesize, 0, inputCodecContext->height, outputFrame->data, outputFrame->linesize);

                    ret = avcodec_send_frame(outputCodecContext, outputFrame);
                    if (ret < 0)
                    {
                        return 18;
                    }

                    while (ret >= 0)
                    {
                        ret = avcodec_receive_packet(outputCodecContext, outputPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        {
                            break;
                        }
                        else if (ret < 0)
                        {
                            return 19;
                        }

                        if (inputFrame && inputFrame->best_effort_timestamp != AV_NOPTS_VALUE)
                        {
                            outputPacket->pts = av_rescale_q(inputFrame->best_effort_timestamp, outputCodecContext->time_base, outputCodecContext->pkt_timebase);
                            outputPacket->dts = av_rescale_q(inputFrame->pkt_dts, outputCodecContext->time_base, outputCodecContext->pkt_timebase);
                        }
                        else
                        {
                            outputPacket->pts = av_rescale_q(outputCodecContext->frame_number, outputCodecContext->time_base, outputCodecContext->pkt_timebase);
                            outputPacket->dts = outputPacket->pts;
                        }

                        fwrite(outputPacket->data, 1, outputPacket->size, outputFile);
                        av_packet_unref(outputPacket);
                    }
                }

                framesProcessed++;
                int64_t percentageComplete = (framesProcessed * 100) / totalFrames;
                std::cout << "Encoding Progress: " << percentageComplete << "%" << std::endl;
            }
            av_packet_unref(outputPacket);
        }

        ret = avcodec_send_frame(outputCodecContext, NULL);
        if (ret < 0)
        {
            return 20;
        }

        while (ret >= 0)
        {
            ret = avcodec_receive_packet(outputCodecContext, outputPacket);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                return 21;
            }

            if (inputFrame && inputFrame->best_effort_timestamp != AV_NOPTS_VALUE)
            {
                outputPacket->pts = av_rescale_q(inputFrame->best_effort_timestamp, outputCodecContext->time_base, outputCodecContext->pkt_timebase);
                outputPacket->dts = av_rescale_q(inputFrame->pkt_dts, outputCodecContext->time_base, outputCodecContext->pkt_timebase);
            }
            else
            {
                outputPacket->pts = av_rescale_q(outputCodecContext->frame_number, outputCodecContext->time_base, outputCodecContext->pkt_timebase);
                outputPacket->dts = outputPacket->pts;
            }

            fwrite(outputPacket->data, 1, outputPacket->size, outputFile);
            av_packet_unref(outputPacket);
        }

        fclose(outputFile);

        av_frame_free(&inputFrame);
        av_frame_free(&outputFrame);
        av_packet_free(&outputPacket);
        avcodec_free_context(&inputCodecContext);
        avcodec_free_context(&outputCodecContext);
        avformat_close_input(&inputFormatContext);
        sws_freeContext(swsContext);

        return 0;
    
}

crow::json::wvalue VideoEncoder::getAvailableCodecs()
{
    const AVCodec *codec = nullptr;
    void *iterator = nullptr;

    crow::json::wvalue obj;

    while ((codec = av_codec_iterate(&iterator)))
    {
        obj[codec->name] = codec->long_name;
        std::cout << "Codec Name: " << codec->name << std::endl;
        std::cout << "Codec Description: " << codec->long_name << std::endl;
        std::cout << std::endl;
    }
    return obj;
}