/***************************************************************************
 *   PALx: A platform independent port of classic RPG PAL                  *
 *   Copyleft (C) 2006 by Pal Lockheart                                    *
 *   palxex@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, If not, see                          *
 *   <http://www.gnu.org/licenses/>.                                       *
 ***************************************************************************/
#include "allegdef.h"

#define BUFFER_SIZE 5040

#define SAMPLE_RATE	44100
#define CHANNELS	1

#define MAX_VOICES 10

bool begin=false,once=false;
int voices[MAX_VOICES];int vocs=0;
void playrix_timer(void *param)
{
	playrix * const plr=reinterpret_cast<playrix*>(param);
	if(voice_get_volume(plr->stream->voice)==0)
		begin=false;
	int f=0;
	for(int i=0;i<vocs;i++)
		if (!voice_check(voices[i])){
			destroy_sample(voice_check(voices[i]));
			std::copy(voices+i+1,voices+MAX_VOICES,voices+i);
			vocs--;
		}
	short *p = (short*)get_audio_stream_buffer(plr->stream);
	if (begin && p)
	{
		static int leaving=0,slen_buf=0,slen=630;
		static short *buf=plr->Buffer;
		if(leaving<BUFFER_SIZE*CHANNELS)
		{
			slen_buf=0;
			int rel=BUFFER_SIZE*CHANNELS-leaving;
			while(slen_buf<rel)
			{
				if(!plr->rix.update())
				{
					if(once)
						plr->stop();
					plr->rix.rewind(plr->subsong);
					continue;
				}
				plr->opl.update(buf, slen);
				for(int t=0;t<slen * CHANNELS;t++)
				{
				    if (*buf >= 16384)
		                        *buf = 32767;
		                    else if (*buf <= -16384)
		                        *buf = -32768;
		                    else
		                        *buf *= 2;
		                    *buf++^=0x8000;
				}
				slen_buf+=slen * CHANNELS;
			 }
			 buf=plr->Buffer;
			 leaving+=slen_buf;
			 buf+=leaving%(BUFFER_SIZE*CHANNELS);
		 }
		 leaving-=BUFFER_SIZE*CHANNELS;
		 memcpy(p,plr->Buffer,BUFFER_SIZE*CHANNELS*2);
		 memcpy(plr->Buffer,plr->Buffer+BUFFER_SIZE*CHANNELS,leaving*2);
		 free_audio_stream_buffer(plr->stream);
	}
	rest(0);
}
END_OF_FUNCTION(playrix_timer);

char playrix::mus[80];
playrix::playrix():opl(SAMPLE_RATE, true, CHANNELS == 2),rix(&opl),Buffer(0),stream(0)
{
	int BufferLength=SAMPLE_RATE*CHANNELS*10;
	rix.load(mus, CProvider_Filesystem());
	stream = play_audio_stream(BUFFER_SIZE, 16, CHANNELS == 2, SAMPLE_RATE, 255, 128);
	LOCK_VARIABLE(Buffer);
	LOCK_VARIABLE(stream);
	LOCK_VARIABLE(opl);
	LOCK_VARIABLE(rix);
	LOCK_VARIABLE(subsong);
	LOCK_FUNCTION(playrix_timer);
	install_param_int(playrix_timer,this,14);

	Buffer = new short [BufferLength];
	memset(Buffer, 0, sizeof(short) * BufferLength);

	voice_set_volume(stream->voice,0);
}
playrix::~playrix()
{
	remove_param_int(playrix_timer,this);
	stop();
	stop_audio_stream(stream);
	delete []Buffer;
}
void playrix::play(int sub_song,int times)
{
	begin=false;
	once=(times==1);
	if(!sub_song){
		subsong=sub_song;
		stop();
		return;
	}
	subsong=sub_song;

	rix.rewind(subsong);
	//opl.init();
	memset(Buffer, 0, sizeof(short) * SAMPLE_RATE * CHANNELS *10);
	memset(stream->samp,0,sizeof(stream->samp));

	begin=true;
	voice_set_volume(stream->voice,1);
	voice_ramp_volume(stream->voice, ((times==3)?2:0)*1000, 255);
}
void playrix::stop(int gap)
{
	voice_ramp_volume(stream->voice, gap*1000, 0);
}
voc::voc(uint8_t *f):spl(load_voc_mem(f))
{}

bool not_voc=false;
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

   if (memcmp(buffer, "Creative Voice File", 0x13)){
	   not_voc=true;
      goto getout;
   }

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
	if(!not_voc)
		voices[vocs++]=play_sample(spl,255,128,1000,0);
}
