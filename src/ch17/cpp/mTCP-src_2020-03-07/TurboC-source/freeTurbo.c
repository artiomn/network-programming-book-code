/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002 Ronald S. Burkey
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
  Contact Ron Burkey at info@sandroid.org.

  Filename:	freeTurbo.c
  Purpose:	This is my custom replacement for the standard library
  		"free" function.
  Mod history:	05/24/02 RSB	Created.
  
  What's the deal with this?  It all has to do with the getimage function
  in graphics.h.  The standard usage of the getimage function (and its
  friends) as envisaged by Borland's docs is this:
  
  1.  Call "imagesize" to find out the size of the image buffer needed.
  2.  Allocate the image buffer with "calloc" or "malloc".
  3.  Fill the image buffer with "getimage".
  4.  Call "putimage" one or more times to write the image somewhere.
  5.  Free the image buffer with "free".
  
  The problem with this is that because our graphics.h functionality 
  works with the X-window system, the graphics data won't *actually*
  be stored in the image buffer, but rather in an X-window Pixmap
  structure of which we have an integer handle, but to which we
  don't have direct access.  So when "getimage" is called, it needs to
  allocate the Pixmap ... but there's nothing in this sequence of operations
  that results in the Pixmap being freed later!
  
  We handle this by adding a little functionality to "free".  "free" checks
  to see if the object being freed is one with which a Pixmap has been
  associated, and if so, frees the Pixmap.  In either case, it then goes
  ahead to free the object's actual memory.
  
  A linked list is maintained of these Pixmaps that have magically been
  created by getimage.
*/

#include "graphics.h"

#ifdef WITH_X

//-------------------------------------------------------------------------
// This is the stuff for the linked list of allocated memory blocks 
// associated with Pixmaps.

struct PixmappedBlock
{
  void *Object;			// Pointer created by malloc/calloc.
  Pixmap Pix;			// The Pixmap's handle.
  struct PixmappedBlock *Next;
};
static struct PixmappedBlock *PixmappedRoot = NULL;

//-------------------------------------------------------------------------
// This is the function which is called by getimage to associate a Pixmap
// with an already-allocated memory block.  If the block is not in the
// list already, it will be added.  If the block *is* in the list, its old
// Pixmap will be freed and then the new one added.  Returns 0 on success,
// non-zero if out of memory.

extern int
AssociatePixmap (void *object, Pixmap handle)
{
  struct PixmappedBlock *Block, *LastBlock;
  // If the list is empty, just add the new block.
  if (PixmappedRoot == NULL)
    {
      PixmappedRoot =
	(struct PixmappedBlock *) malloc (sizeof (struct PixmappedBlock));
      if (PixmappedRoot == NULL)
	return (1);
      PixmappedRoot->Object = object;
      PixmappedRoot->Pix = handle;
      PixmappedRoot->Next = NULL;
      return (0);
    }
  // Navigate the linked list to see if this block is already in it!
  // Note that this is guaranteed to iterate at least once, so if it falls
  // through, we know that LastBlock has been initialized.
  for (Block = PixmappedRoot; Block != NULL;
       LastBlock = Block, Block = Block->Next)
    if (Block->Object == object)
      {
	if (Block->Pix != handle)
	  {
	    // The block *is* already defined, but is associated with
	    // a different Pixmap.  So free the old Pixmap and associate
	    // a new one!
	    XFreePixmap (TcDisplay, Block->Pix);
	    Block->Pix = handle;
	  }
	return (0);
      }
  // The block wasn't in the list, so add it.
  Block = (struct PixmappedBlock *) malloc (sizeof (struct PixmappedBlock));
  if (Block == NULL)
    return (1);
  Block->Object = object;
  Block->Pix = handle;
  Block->Next = NULL;
  LastBlock->Next = Block;
  return (0);
}

//-------------------------------------------------------------------------
// This is the replacement for "free".  Free the allocated object, as usual,
// but if there's a Pixmap associated with the object, then free the Pixmap
// and remove the Pixmap-association entry in the linked list.

extern void
freeTurbo (void *object)
{
  struct PixmappedBlock *Block, *LastBlock;
  // We can free the regular memory right away, because we don't actually
  // use it for anything below; we're only interested in its pointer.
  freeUnix (object);
  // Check if the list is empty.
  if (PixmappedRoot == NULL)
    return;
  // Navigate the linked list to see if this block is in it!
  for (Block = PixmappedRoot; Block != NULL;
       LastBlock = Block, Block = Block->Next)
    if (Block->Object == object)
      {
	XFreePixmap (TcDisplay, Block->Pix);
	if (Block == PixmappedRoot)
	  PixmappedRoot = Block->Next;
	else
	  LastBlock->Next = Block->Next;
	freeUnix (Block);
	return;
      }
}

//------------------------------------------------------------------------
// If not using X-window system, the whole point is moot ...

#else // WITH_X

extern void
freeTurbo (void *object)
{
  freeUnix (object);
}

#endif // WITH_X
