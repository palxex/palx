#include "allegdef.h"

#define BUFFER_SIZE 8192

#define SAMPLE_RATE	48000
#define CHANNELS	2

void playrix_timer(void *param)
{
	playrix *plr=reinterpret_cast<playrix*>(param);
	short *p = (short*)get_audio_stream_buffer(plr->stream);
	if (p) 
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

playrix::playrix():opl(SAMPLE_RATE, true, CHANNELS == 2),rix(&opl),leaving(0),tune(0),Buffer(0),stream(0)
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
}
playrix::~playrix()
{
	stop();
	stop_audio_stream(stream);
}
void playrix::play(int sub_song)
{
	stop();
	int slen = rix.songlength(sub_song);
	subsong=sub_song;
	BufferLength = SAMPLE_RATE * CHANNELS / 1000 * slen;
	Buffer = buf = new short [BufferLength];
	memset(buf, 0, sizeof(short) * BufferLength);
	
	install_param_int(playrix_timer,this,14);
}
void playrix::stop()
{
	remove_param_int(playrix_timer,this);
	if(Buffer)
		delete[] Buffer;
}