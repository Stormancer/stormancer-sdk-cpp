FROM ducatel/visual-studio-linux-build-box

ENV HOME /root
WORKDIR $HOME 

RUN apt-get update && \
	apt-get install -y rsync p7zip-full clang-7 libc++-7-dev libc++abi-7-dev

COPY ue422sdk.7z .
RUN 7za x ue422sdk.7z

