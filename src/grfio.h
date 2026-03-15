#ifndef	_GRFIO_H_
#define	_GRFIO_H_

void grfio_init(void);		// GRFIO Initialize
int grfio_add(char*);		// GRFIO Resource file add
void* grfio_read(char*);	// GRFIO data file read
int grfio_size(char*);		// GRFIO data file size get

#endif	// _GRFIO_H_
