/*
  This code contains an implementation of a simple note synthesizer.
   
  It was originally created for CSC B63, and has now been updated
  for use in A48. 
    
  *** YOU DO NOT NEED TO READ OR UNDERSTAND THE CODE HERE ***
  
  It is provided only to complement the assignment you're working
  on.
  
  However - if you are curious about how to generate sound and
  how to write a sound file, feel free to study the code below,
  ONLY AFTER YOU:
    
       - Have completed and tested your assignment solution
       - Have made sure you have time for this!
       
  If you're curious about sound, sound generation, or any of the
  code in here, feel free to drop by and I can help explain
  what's going on during the office hour.
  
  (c) F. Estrada, Feb. 2019       
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<math.h>

#define FS 44100			// Defines the sampling frequency for note generation
					// in Hz. that means, we will generate 44100 samples
					// for each note per second. This is the number of
					// samples needed for CD-quality sound.
					// Question: What is sampling anyway?

typedef struct note_struct{		// This holds the data representing a single note
	double freq;			// This note's frequency
	int bar;			// WHich bar in the song this note belongs to
	double index;			// Time index within the bar (from 0 to 1.0)
					// Note that the BST uses (bar*10)+index to sort notes, so
					// technically we can't have 2 notes at exactly the same bar
					// and index. Simply give these 2 notes the same bar, and
					// index values that are *very* close together, but not
					// identical, to achieve the same effect as having the
					// notes play at the same time index
	double *waveform;		// A pointer to a dynamically generated array which will contain
					// the note's waveform
        double out_tminus1;		// Output at t-1
	int wave_length;		// Length of the waveform in samples
	int input_idx, output_idx;	// Indices used to access data in the waveform array
	int n_sampled;			// Counter indicating how many samples have been generated for
					// this note. Used to stop playing notes after a specified duration
					// has been reached.
	struct note_struct *next;	// Since notes are created dynamically, we'll place them on a linked
					// list.
} note;

// GLOBAL data
note    *playlist_head=NULL;		// GLOBAL variable for the playlist!
char    note_names[100][5];		// Array of note names, there are 100 of them
double  note_freq[100];			// Array of note frequencies, one per note name

void read_note_table(void)
{
    // Read note names and frequencies from file
    FILE *f;
    char line[1024];
    int idx;
   
    f=fopen("note_frequencies.txt","r");
    if (f==NULL)
    {
      printf("Unable to open note frequencies file!\n");
      return;
    }
    
    idx=0;
    while (fgets(&line[0],1024,f))
    {
      note_names[idx][0]=line[0];
      note_names[idx][1]=line[1];
      if (line[2]!='\t') note_names[idx][2]=line[2]; else note_names[idx][2]='\0';
      note_names[idx][3]='\0';

      if (line[2]=='\t') note_freq[idx]=strtod(&line[3],NULL);
      else note_freq[idx]=strtod(&line[4],NULL);
      
      printf("Read note for table with name %s, frequency=%f\n",&note_names[idx][0],note_freq[idx]);
      idx++;
    }
    printf("Processed %d notes!\n",idx);
}

note *new_note(double freq, int bar, double index)
{
 // This function creates an initializes a new note from the input values
 // that describe the note's frequency, and its position in the song
 // bar:index
 note *n;
 
 n=(note *)calloc(1,sizeof(note));
 if (!n)
 {
  fprintf(stderr,"new_note(): Out of Memory!, are you trying to load a Great Symphony???\n");
  return(NULL);
 }
 
 // Set up the note's internal data
 n->freq=freq;
 n->bar=bar;
 n->index=index;
 n->input_idx=0;
 n->output_idx=1;
 n->out_tminus1=((double)rand()/(double)RAND_MAX)-.5;
 n->n_sampled=0;
 n->next=NULL;

 // We need to set up a dynamically allocated array to hold the note's waveform. 
 // The length of this array must correspond to the number of samples it takes to
 // represent a waveform with the note's frequency.
 //
 // The formula is length=sampling_rate/note_frequency. 
 // e.g. a 440Hz note (A4) at a sampling rate of 44100Hz requires 44100/440=100.22 (round to nearest)
 // samples to represent, and so, we would create an array with 100 entries for its waveform

 n->wave_length=round((double)FS/n->freq);
 n->waveform=(double *)calloc(n->wave_length,sizeof(double));
 if (!n)
 {
  fprintf(stderr,"new_note(): Ran out of memory allocating waveform. Toast!\n");
  return(NULL);
 }

 //////////////////////////////////////////////////////////////////////////////////////////////
 // Here we initialize the note's waveform. 
 //
 // You could potentially change the note sound by playing around with different initialization
 // techniques. For example, what happens if you use a square wave as an initial waveform?
 // a ramp? a sine wave? Note that all waveform values must be in [-1, 1]
 // 
 // For the Karplus-Strong plucked string, the standard initialization is white noise.
 //////////////////////////////////////////////////////////////////////////////////////////////
 for (int i=0;i<n->wave_length;i++)
 {
  *(n->waveform + i)=(1.0*(double)rand()/(double)RAND_MAX)-.5;	// A random number in [-.5, .5]
 }

 return(n);
}

note *playlist_insert(note *head, double freq, int bar, double index)
{

 // This function adds a new note at the requested location. The node is
 // added at the end of the linked list, so this assumes the notes are
 // already sorted in sequential order of bar:index. If this is not
 // true, weird things may happen during playback.
  
 note *n_n, *p;
 
 n_n=new_note(freq,bar,index);
 if (n_n==NULL) return head;
 
 if (head==NULL) return n_n;

 p=head;
 while (p->next!=NULL) p=p->next;
 p->next=n_n;
 return head;  
}

void delete_playlist(note *head)
{
   // Release memory allocated for a playlist
   note *t;
   
   while (head!=NULL)
   {
     t=head->next;
     free(head->waveform);
     free(head);
     head=t;
   }
}

int key_from_time_index(int bar, int index)
{
 /*
   This function creates a unique int key for a note from bar and index. Note that
   duplicated notes (same note, same time index) will be handled by the tree
   insertion process. 
 */

 int key=0;

 key=(bar*100) + (index+1);			// Orders notes by bar:index
 return(key);
}

double KS_string_sample(note *n)
{
 /*
    This function implements the Karplus-Strong plucked string
    algorithm. Karplus-Strong belongs to the class of sound synthesis
    algorithms that attempt to simulate the physical behaviour of
    the sound-producing components of a musical instrument.
 
    The Karplus-Strong method is in effect a simple simulation of the
    behaviour and propagation of vibrations along a string. It has
    two main components:
  
    A delay line which represents the vibrating string. The length
    of the delay line must be equivalent to the wavelength at which
    the string vibrates.

    A feedback loop consisting of a low-pass filter. There are many
    ways to set up this filter, here we use the simplest form which
    averages two consecutive samples and feeds them back to the
    input with a gain less than 1.

    The delay line works like this:
    * At each time step, the right-most entry in the delay line is sent
      to the output. The contents of the entire delay line are shifted
      one space to the right, and the input for this time step is
      stored at the left-most space in the delay line.
    * The 'input' in this case comes from the feedback loop.

    A simple schematic of KS is shown below:


			Delay line of length n,
			sound samples move to the
 			right by one space at each
			step
                -------------------------------------
     --- in --->|  |  |  |  |  |  |  |  |  |  |  |  |-------------> out
     |          -------------------------------------      |
     |							   |
     |			delay of length 1		   |
   (*.995)  			 _			   |
     |			   /----|_|----\                   |
     |                     |           |                   |
     \-------------------- + ------------------------------/
		Here the new input is
		computed as a weighted sum of
                the current and previous output
		and then multiplied by .995 to
		avoid self-amplifying feedback			

   Of course, displacing all data in the delay line would be slow,
   instead. The delay line is implemented as a circular array and two
   pointers are used to keep track of the current input and output.

   Much more complex physical models exist. If you are curious, 
   visit the Stanford University's Center for Computer Research in
   Music and Acoustics (CCRMA). And look for 'Virtual Acoustic
   Synthesis' and 'waveguides'. 

   Enjoy the sound of plucked strings!

   Incidentally, most software synthesizers rely on recordings of
   instrument waveforms which are played back at the appropriate
   frequency and over which effects and processing are performed.
   Physical simulation is still limited by the complexity and
   computational cost of its models.
 
   If you are curious about how modern software synthesizers work,
   read up on 'wavetable synthesis'.
 */

 double output, new_input;

 n->n_sampled++;		// Keep track of how many times we've sampled this note
				// to see if we have exceeded the note's duration

 // Get output
 output=*(n->waveform+n->output_idx);
 new_input=.995*((.25*n->out_tminus1)+(.75*output));

 // Update internal state
 n->out_tminus1=output;
 *(n->waveform+n->input_idx)=new_input;

 n->output_idx++;
 if (n->output_idx==n->wave_length) n->output_idx=0;
 n->input_idx++;
 if (n->input_idx==n->wave_length) n->input_idx=0;

 return(output);
}

void write_wav_header(FILE *f, unsigned int samples)
{
 /*
   Writes out the .wav header for the output file.
 */
 char txt[250];
 uint32_t chunk2size;
 uint32_t tmp;
 unsigned char tmp2;
 // Compute the chunk size in bytes
 chunk2size=samples*2*16/8;			// # samples, 2 channels, 16 bits/channel, 8 bits per byte

 strcpy(&txt[0],"RIFF");			// RIFF header
 fwrite(&txt[0],4*sizeof(unsigned char),1,f);
 tmp=chunk2size+36;
 fwrite(&tmp,4*sizeof(unsigned char),1,f);	// You will have to fix this for 64bit systems!
 strcpy(&txt[0],"WAVE");
 fwrite(&txt[0],4*sizeof(unsigned char),1,f);	// Wave file identifier
 strcpy(&txt[0],"fmt ");
 fwrite(&txt[0],4*sizeof(unsigned char),1,f);	// format section
 tmp=16;
 fwrite(&tmp,4*sizeof(unsigned char),1,f);	// Because I said so...
 tmp2=1;
 fwrite(&tmp2,sizeof(unsigned char),1,f);
 tmp2=0;		
 fwrite(&tmp2,sizeof(unsigned char),1,f);	// Format identifier (01) for linear PCM (remember Endianness??)
 tmp2=2;
 fwrite(&tmp2,sizeof(unsigned char),1,f);
 tmp2=0;		
 fwrite(&tmp2,sizeof(unsigned char),1,f);	// Number of channels (02) for two channel stereo
 tmp=FS;
 fwrite(&tmp,4*sizeof(unsigned char),1,f);	// Sampling rate
 tmp=FS*2*16/8;					
 fwrite(&tmp,4*sizeof(unsigned char),1,f);	// Byte rate, sampling rate * num channels * bits/channel / 8 bits
 tmp2=2*16/8;
 fwrite(&tmp2,sizeof(unsigned char),1,f);
 tmp2=0;
 fwrite(&tmp2,sizeof(unsigned char),1,f);	// Block alignment = num_channels * bits/channel / 8 bits
 tmp2=16;
 fwrite(&tmp2,sizeof(unsigned char),1,f);
 tmp2=0;
 fwrite(&tmp2,sizeof(unsigned char),1,f);	// Bits per sample - 16
 strcpy(&txt[0],"data");
 fwrite(&txt[0],4*sizeof(unsigned char),1,f);	// data field follows...
 fwrite(&chunk2size,4*sizeof(unsigned char),1,f);	// But of course, first let's insert the chunk2size! (why not?)
 // Data follows this portion... 
}

void play_notes(int bar_length)
{
 /*
   This is the main synthesis function. It starts a time counter from bar=1, idex=0
   and plays out the notes in the song at the specified times. Notes are placed in
   a temporary linked list and pointers are used to keep track of which notes are
   active at any given moment.

   Sound synthesis is done through each note's next_sample() function, which 
   returns the value of the waveform for that note for each sampling step.   
   
   bar_length indicates the duration in seconds for each bar, and controls the
   overall speed of playback.
 */
 note *q, *st, *ed;
 int bar, max_bar, max_length, s_idx;
 double index;
 unsigned int sample_idx, max_sample_idx;
 int16_t this_sample_int;
 double this_sample, this_sample_r, this_sample_l;
 double balance,fAmp;
 int done;
 FILE *f;

 if (playlist_head==NULL) 
 {
   printf("Input playlist is empty!\n");
   return;
 }
 max_bar=0;
 q=playlist_head;
 while (q!=NULL)
 {
  max_bar=q->bar;
  q=q->next;
 }

 // Let's play!
 max_length=3*FS;		// Maximum note length, number of seconds * sampling rate.
				// A more complex synthesizer would allow each note to have a different lentgh.
				// We would need to indicate note lengths in the input file and store them in
				// the note data structure

 // Calculate song length in samples 
 max_sample_idx=bar_length*FS*(max_bar+1);

 fprintf(stderr,"\nPlaying song. %d bars, max note length is %d samples. Output is 'output.wav'\n\n",max_bar,max_length); 

 f=fopen("output.wav","wb+");	// Open file 'output.wav' for writing. Mind the file type if you work on Windows
 if (f)
 {
  write_wav_header(f,max_sample_idx);

  // Main loop of the synthesis part. Keeps track of bar:index position and plays any notes
  // that are active within the interval. This includes notes that start at this index,
  // and previously active notes that have not finished playing. 
  // For each sample, we ask each note to provide a sample via its next_sample() function
  // which is tied to the appropriate synthesis algorithm

  st=ed=playlist_head;			// Start and end pointers indicating range of active notes
					// ed is manipulated via the bar:index combination.
					// st moves forward when notes complete playing.
  sample_idx=0;				// Global sample index, used to terminate if for some
					// reason a note goes beyond our estimate for max_sample_idx
  for (bar=playlist_head->bar;bar<=max_bar;bar++)
  {
   for (index=0;index<=1.0;index+=1.0/(bar_length*44100.0))
   {
    // Check if we have to update ed (i.e. we have to add notes for this time index)
    done=0;
    while(!done)		
    {
     done=1;
     if (ed->next!=NULL) 
     {
       if ((double)ed->next->bar+ed->next->index<=(double)bar+index) 
       {
	 ed=ed->next;
	 done=1;
       }
     }
    }
    
    // For each note between st and ed, generate any needed samples, and output to file
    // Note that at this point it would be MUCH more efficient to have a buffer where to
    // store temporary output data, and write to disk only a few times instead of once
    // for every sample. For simplicity, we'll write directly, and use this as a chance
    // to see how good the OS is as buffering file output by itself.

    // Need to generate all the samples for this index
    if (st!=NULL)
    {
      q=st;
      this_sample_l=0;
      this_sample_r=0;
      while(ed->next!=q)
      {
       balance=(q->freq-525.0);			// C5 at center
       if (balance<0) balance=(525.0+balance)/525.0;
       else balance=balance/(5000.0-525.0);
       fAmp=(pow(q->freq,.33)/18.0);		// Amplitude boost for high freq. notes
       this_sample=KS_string_sample(q)*fAmp;
       this_sample_l+=(balance)*this_sample;
       this_sample_r+=(1.0-balance)*this_sample;
       q=q->next;
      } 
      
      //////////////////////////////////////////////////////////////////////////////////
      //   Note that I use position in the scale as a way to separate notes
      ///  across the L and R channels. What else could we think of doing here?
      //////////////////////////////////////////////////////////////////////////////////

      // Clip to [-1, 1] transform to a 2-byte signed integer for each channel, and write
      // to file
      this_sample_r=tanh(this_sample_r);
      this_sample_int=(int16_t)(this_sample_r*32700);
      fwrite(&this_sample_int,2*sizeof(unsigned char),1,f);    	// R channel
      this_sample_l=tanh(this_sample_l);
      this_sample_int=(int16_t)(this_sample_l*32700);
      fwrite(&this_sample_int,2*sizeof(unsigned char),1,f);    	// L channel

      sample_idx++;
    }
    else	// No more notes to play, fill out the remainder of the file with zeros.
    {
     // Generate all samples with zeros
     for (s_idx=0;s_idx<44100;s_idx++)
     {
      this_sample_int=0;
      fwrite(&this_sample,2*sizeof(unsigned char),1,f);    	// L channel
      fwrite(&this_sample,2*sizeof(unsigned char),1,f);    	// R channel
      sample_idx++;
     }
    }
 
    // Check if notes have ended 
    done=0;
    while (!done)
    {
     done=1;
     if (st!=NULL&&st!=ed->next)				// Should never go beyond ed
      if (st->n_sampled>max_length) 
      {
       st=st->next;
       done=0;
      }       
    }
    if (sample_idx>max_sample_idx) break;
   }		// End for index...
   if (sample_idx>max_sample_idx) break;
  }		// End for bar...
	
  fclose(f);
 }
 else fprintf(stderr,"Unable to open file for output!\n");

 delete_playlist(playlist_head);
 playlist_head=NULL;
}

