# Helix-mp3 files.
HELIXMP3SRC = $(CHIBIOS)/ext/libhelix-mp3/mp3dec.c \
              $(CHIBIOS)/ext/libhelix-mp3/mp3tabs.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/bitstream.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/buffers.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/dct32.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/dequant.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/dqchan.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/huffman.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/hufftabs.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/imdct.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/polyphase.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/scalfact.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/stproc.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/subband.c \
              $(CHIBIOS)/ext/libhelix-mp3/real/trigtabs.c 
              
HELIXMP3ASMSRC = $(CHIBIOS)/ext/libhelix-mp3/real/arm/asmmisc.s \
                 $(CHIBIOS)/ext/libhelix-mp3/real/arm/asmpoly_gcc.S \
                 $(CHIBIOS)/ext/libhelix-mp3/real/arm/asmpoly.s

HELIXMP3INC = $(CHIBIOS)/ext/libhelix-mp3 \
              $(CHIBIOS)/ext/libhelix-mp3/real \
              $(CHIBIOS)/ext/libhelix-mp3/pub
              
DDEFS += -DARM -DARM7DI

# Shared variables
ALLCSRC += $(HELIXMP3SRC)
ASMSRC += $(HELIXMP3ASMSRC)
ALLINC  += $(HELIXMP3INC)
