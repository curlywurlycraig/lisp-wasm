FROM alpine:3.14.1

RUN apk update && apk add alpine-sdk clang entr
