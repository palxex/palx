#include "allegdef.h"

#define BUFFER_SIZE 4096

#define SAMPLE_RATE	48000
#define CHANNELS	1

void playrix_timer(void *param)
{
	playrix *plr=reinterpret_cast<playrix*>(param);
	short *p = (short*)get_audio_stream_buffer(plr->stream);
	if (plr->playing && p) 
	{
		if(plr->leaving<BUFFER_SIZE*CHANNELS)
		{
			plr->slen_buf=0;
			int rel=BUFFER_SIZE*CHANNELS-plr->leaving;
			while(plr->slen_buf<rel)
			{
				if(!plr->rix.update())
				{
					plr->rix.rewind(plr->subsong);
					continue;
				}
				plr->slen = SAMPLE_RATE / plr->rix.getrefresh();
				plr->opl.update(plr->buf, plr->slen);
				for(int t=0;t<plr->slen * CHANNELS;t++)
					*plr->buf++^=0x8000;
				plr->slen_buf+=plr->slen * CHANNELS;
			 }
			 plr->buf=plr->Buffer;
			 plr->leaving+=plr->slen_buf;
			 plr->buf+=plr->leaving%(BUFFER_SIZE*CHANNELS);
			 plr->tune=0;
		 }
		 plr->leaving-=BUFFER_SIZE*CHANNELS;
		 memcpy(p,plr->Buffer,BUFFER_SIZE*CHANNELS*2);
		 memcpy(plr->Buffer,plr->Buffer+BUFFER_SIZE*CHANNELS,plr->leaving*2);
		 free_audio_stream_buffer(plr->stream);
	}
}
END_OF_FUNCTION(playrix::playrix_timer);

playrix::playrix():opl(SAMPLE_RATE, true, CHANNELS == 2),rix(&opl),leaving(0),tune(0),Buffer(0),stream(0),playing(false)
{
	rix.load(std::string("mus.mkf"), CProvider_Filesystem());
	stream = play_audio_stream(BUFFER_SIZE, 16, CHANNELS == 2, SAMPLE_RATE, 255, 128);
	LOCK_VARIABLE(Buffer);
	LOCK_VARIABLE(leaving);
	LOCK_VARIABLE(slen);
	LOCK_VARIABLE(slen_buf);
	LOCK_VARIABLE(tune);
	LOCK_VARIABLE(stream);
	LOCK_VARIABLE(opl);
	LOCK_VARIABLE(rix);
	LOCK_VARIABLE(subsong);
	LOCK_FUNCTION(playrix_timer);
	install_param_int(playrix_timer,this,14);
	
	BufferLength = SAMPLE_RATE * CHANNELS * 3;
	Buffer = buf = new short [BufferLength];
	memset(buf, 0, sizeof(short) * BufferLength);
}
playrix::~playrix()
{
	stop();	
	remove_param_int(playrix_timer,this);
	stop_audio_stream(stream);
	delete []Buffer;
}
void playrix::play(int sub_song)
{
	if(subsong==sub_song)
		return;
	voice_start(stream->voice);
	subsong=sub_song;
	rix.rewind(subsong);
	
	playing=true;
}
void playrix::stop()
{
	playing=false;
	voice_stop(stream->voice);
}

voc::voc(uint8_t *f):spl(load_voc_mem(f))
{}

voc::~voc()
{
	destroy_sample(spl);
}

SAMPLE *voc::load_voc_mem(uint8_t *src)
{
   char buffer[30];
   int freq = 22050;
   int bits = 8;
   uint8_t *f=src;
   SAMPLE *spl = NULL;
   int len;
   int x, ver;
   int s;
   ASSERT(f);

   memset(buffer, 0, sizeof buffer);

   //pack_fread(buffer, 0x16, f);
   memcpy(buffer,f,0x16);f+=0x16;

   if (memcmp(buffer, "Creative Voice File", 0x13))
      goto getout;

   ver = ((uint16_t*)f)[0];f+=2;
   if (ver != 0x010A && ver != 0x0114) /* version: should be 0x010A or 0x0114 */
      goto getout;

   ver = ((uint16_t*)f)[0];f+=2;
   if (ver != 0x1129 && ver != 0x111f) /* subversion: should be 0x1129 or 0x111f */
      goto getout;

   ver = f[0];f++;
   if (ver != 0x01 && ver != 0x09)     /* sound data: should be 0x01 or 0x09 */
      goto getout;

   len = ((uint16_t*)f)[0];f+=2;                /* length is three bytes long: two */
   x = f[0];f++;                   /* .. and one byte */
   x <<= 16;
   len += x;

   if (ver == 0x01) {                  /* block type 1 */
      len -= 2;                        /* sub. size of the rest of block header */
      x = f[0];f++;                /* one byte of frequency */
      freq = 1000000 / (256-x);

      x = f[0];f++;                /* skip one byte */

      spl = create_sample(8, FALSE, freq, len);

      if (spl) {
	 //if (pack_fread(spl->data, len, f) < len) {
	  memcpy(spl->data,f,len);f+=len;
	 //   destroy_sample(spl);
	 //   spl = NULL;
	 //}
      }
   }
   else {                              /* block type 9 */
      len -= 12;                       /* sub. size of the rest of block header */
      freq = ((uint16_t*)f)[0];f+=2;            /* two bytes of frequency */

      x = ((uint16_t*)f)[0];f+=2;               /* skip two bytes */

      bits = f[0];f++;             /* # of bits per sample */
      if (bits != 8 && bits != 16)
	 goto getout;

      x = f[0];f++;
      if (x != 1)                      /* # of channels: should be mono */
	 goto getout;

      //pack_fread(buffer, 0x6, f);      /* skip 6 bytes of unknown data */
	  f+=6;

      spl = create_sample(bits, FALSE, freq, len*8/bits);

      if (spl) {
	 if (bits == 8) {
	 //   if (pack_fread(spl->data, len, f) < len) {
		  memcpy(spl->data,f,len);f+=len;
	 //      destroy_sample(spl);
	 //      spl = NULL;
	 //   }
	 }
	 else {
	    len /= 2;
	    for (x=0; x<len; x++) {
	 //      if ((s = pack_igetw(f)) == EOF) {
			s=((uint16_t*)f)[0];f+=2;
	 //	  destroy_sample(spl);
	//	  spl = NULL;
	//	  break;
	//       }
	       ((signed short *)spl->data)[x] = (signed short)s^0x8000;
	    }
	 }
     }
   }

getout:
   return spl;
}

void voc::play()
{
	play_sample(spl,255,128,1000,0);
}
