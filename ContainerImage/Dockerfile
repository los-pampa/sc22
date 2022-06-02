FROM ubuntu:20.04
COPY ./gcc.zip /home
RUN apt update && apt upgrade -y && apt install -y python make unzip && apt update && apt autoremove -y && mkdir /opt/gcc && cd /home && mv gcc.zip /opt/gcc && cd /opt/gcc && unzip gcc.zip && rm gcc.zip
ENV PATH="/opt/gcc/build/bin:${PATH}"
ENV LD_LIBRARY_PATH="/opt/gcc/build/lib64:${LD_LIBRARY_PATH}"
