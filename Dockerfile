FROM alpine:3.20 AS builder

RUN apk add --no-cache g++ cmake make git openssl-dev zlib-dev linux-headers python3 gperf

WORKDIR /build
COPY . .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)

FROM alpine:3.20

RUN apk add --no-cache libstdc++ openssl zlib

COPY --from=builder /build/build/telegram-bot-api /usr/local/bin/telegram-bot-api

EXPOSE 8081
ENTRYPOINT ["telegram-bot-api"]
CMD ["--enable-file-streaming", "--http-port=8081"]
