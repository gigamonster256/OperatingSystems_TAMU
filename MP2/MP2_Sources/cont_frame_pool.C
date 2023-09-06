/*
 File: ContFramePool.C
 
 Author: Caleb Norton 628007801
 Date  : 9/5/2023
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

// init the static variable
ContFramePool *ContFramePool::frame_pool_list_head = NULL;

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
	// make sure that we will own at least one frame
	assert(_n_frames > 0);

	// the number of frames that we need to store state
	// must be at least 1 less than the number of frames
	// that we will manage otherwise, all the frames
	// would be used to store state and none would be
	// available for allocation
	unsigned long info_frames_needed = needed_info_frames(_n_frames);
	
	// verify that we have enough space
	assert(info_frames_needed < _n_frames);

	// save info about frame pool
	info_frame_no = _info_frame_no;
	base_frame_no = _base_frame_no;
	n_frames = _n_frames;
	next = frame_pool_list_head;

	// update the list head
	// (this is a static variable, so it is shared by all instances)
	frame_pool_list_head = this;

	// 'allocate' the info frame(s)
	if (info_frame_no == 0) {
		// if _info_frame_no is 0, then we need to allocate the first
		// frames as info frames
		bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
		// if we own the info frames, we need to mark them as Inaccessible
		mark_inaccessible(base_frame_no, info_frames_needed);
	} else {
		// otherwise use the address we were given (allocated elsewhere)
		bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
	}
}

ContFramePool::~ContFramePool()
{
	// iterate over the info list and remove the ourselves
	ContFramePool *curr = frame_pool_list_head;
	ContFramePool *prev = NULL;
	while (curr != NULL) {
		if (curr == this) {
			if (prev == NULL) {
				// we are the head of the list
				frame_pool_list_head = curr->next;
			} else {
				// we are not the head of the list
				prev->next = curr->next;
			}
			break;
		}
		prev = curr;
		curr = curr->next;
	}
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
	// make sure we are allocating at least one frame   
	assert(_n_frames > 0);

	// relative frame number of the first frame in the sequence
	unsigned long i = 0;
seek:	
	// verify we are still not nearing the end of the frame pool
	if (i + _n_frames > n_frames) {
		return 0;
	}
		
	// if the frame isn't free, skip it
	if (get_state(i) != FrameState::Free) {
		i++;
		goto seek;
	}
		
	// the frame is free, verify the next _n_frames - 1 frames are free
	for (unsigned int j = 1; j < _n_frames; j++) {
		// check that the frame is free
		if (get_state(i + j) != FrameState::Free) {				
			// if it is not free, we need to look further
			i += j + 1;
			goto seek;
		}
	}

	// if we get here, we found a sequence of free frames
	// mark the first frame as HeadOfSequence
	set_state(i, FrameState::HeadOfSequence);
	// and the remaining frames as Used
	for (unsigned int j = 1; j < _n_frames; j++) {
		set_state(i + j, FrameState::Used);
	}

	// return the absolute frame number of the first frame
	return i + base_frame_no;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
	// assert we own the frames
	assert(_base_frame_no >= base_frame_no &&
			_base_frame_no + _n_frames < base_frame_no + n_frames);

	for (unsigned long i = 0; i < _n_frames; i++) {
		// do a little math to get the relative frame number
		unsigned long relative_frame_no = _base_frame_no - base_frame_no + i;
		set_state(relative_frame_no, FrameState::Inaccessible);
	}
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
	// find the frame pool that owns the frame
	ContFramePool *frame_pool = find_frame_pool(_first_frame_no);

	// assert that we found a frame pool
	assert(frame_pool != NULL);

	// release the frames
	frame_pool->release_frames_internal(_first_frame_no);
}

void ContFramePool::release_frames_internal(unsigned long _first_frame_no)
{
	// verify we own the frame
	assert(_first_frame_no >= base_frame_no &&
			_first_frame_no < base_frame_no + n_frames);

	// calculate the relative frame number
	unsigned long relative_frame_no = _first_frame_no - base_frame_no;

	// verify that the frame is the head of a sequence
	assert(get_state(relative_frame_no) == FrameState::HeadOfSequence);

	// mark the frame and all following Used frames as Free
	unsigned long i = relative_frame_no;
	do {
		set_state(i, FrameState::Free);
		i++;

		// if we are at the end of the frame pool, we are done
		if (i >= n_frames) {
			break;
		}
	} while (get_state(i) == FrameState::Used);
}

// this should turn into a single expression upon compilation
unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
	// for every frame, we need 2 bits 
	unsigned long bits_needed_for_info = 2 * _n_frames;

	// and the amount of frames the info will take is
	unsigned long info_frames_needed = bits_needed_for_info / (FRAME_SIZE * 8);

	// if there is a remainder, we need one more frame
	if (bits_needed_for_info % (FRAME_SIZE * 8) != 0) {
		info_frames_needed++;
	}

	return info_frames_needed;
}

ContFramePool::FrameState ContFramePool::get_state(unsigned long _rel_frame_no)
{
	// assert that we own the frame
	assert(_rel_frame_no < n_frames);

	// get the location of the frame
	unsigned int bitmap_index = _rel_frame_no / 4;
	unsigned char mask = 0b11 << ((_rel_frame_no % 4) * 2);

	char state = (bitmap[bitmap_index] & mask) >> ((_rel_frame_no % 4) * 2);

	switch (state) {
		case 0b00:
			return FrameState::Free;
		case 0b01:
			return FrameState::HeadOfSequence;
		case 0b10:
			return FrameState::Used;
		case 0b11:
			return FrameState::Inaccessible;
		default:
			assert(false);
			// this is just to make the compiler happy
			return FrameState::Free;
	}
}

void ContFramePool::set_state(unsigned long _rel_frame_no, FrameState _state)
{
	// assert that we own the frame
	assert(_rel_frame_no < n_frames);

	// get the location of the frame
	unsigned int bitmap_index = _rel_frame_no / 4;
	unsigned char mask = 0b11 << ((_rel_frame_no % 4) * 2);

	// clear the bits
	bitmap[bitmap_index] &= ~mask;

	// set the bits
	switch (_state) {
		case FrameState::Free:
			// do nothing
			break;
		case FrameState::HeadOfSequence:
			bitmap[bitmap_index] |= 0b01 << ((_rel_frame_no % 4) * 2);
			break;
		case FrameState::Used:
			bitmap[bitmap_index] |= 0b10 << ((_rel_frame_no % 4) * 2);
			break;
		case FrameState::Inaccessible:
			bitmap[bitmap_index] |= 0b11 << ((_rel_frame_no % 4) * 2);
			break;
		default:
			assert(false);
	}
}
	
ContFramePool *ContFramePool::find_frame_pool(unsigned long _frame_no){
	// iterate over the info list and get the frame pool
	// that has ownership of the frame
	ContFramePool *curr = frame_pool_list_head;
	while (curr != NULL) {
		if (_frame_no >= curr->base_frame_no &&
				_frame_no < curr->base_frame_no + curr->n_frames) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}
