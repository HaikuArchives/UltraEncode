#include <DataIO.h>


#ifndef _included_Attribute_h
#define _included_Attribute_h

/**
This class provides a friendly representation of an attribute, based on BPositionIO.
Data is manipulated in memory until Flush()ed out to disk.

* @author Stephen van Egmond
*/
class Attribute
	/* extends */ :public BPositionIO
{
public:
	/** Opens (or creates) an attribute of the given name and type on a given node.
		Refer to the BNode class to see the significance of the Attribute
		* @param node The node to open the attribute on.
		* @param name The name of the attribute.
		* @param type One of the B_*_TYPE constants.
	*/		
	Attribute(const BNode *node, const char *name, bool createNew);

	~Attribute();
	
	/// Inherited from BPositionIO.
	virtual	ssize_t		Read(void *buffer, size_t size);

	/// Inherited from BPositionIO.
	virtual	ssize_t		Write(const void *buffer, size_t size);

	/// Inherited from BPositionIO.
	virtual	ssize_t		ReadAt(off_t pos, void *buffer, size_t size);

	/// Inherited from BPositionIO.
	virtual	ssize_t		WriteAt(off_t pos, const void *buffer, size_t size);

	/// Inherited from BPositionIO.
	virtual off_t		Seek(off_t position, uint32 seek_mode);

	/// Inherited from BPositionIO.
	virtual	off_t		Position() const;

	/// Inherited from BPositionIO.
	virtual status_t	SetSize(off_t size);



	/// Writes the attribute data in memory out to disk.
	void Flush();

	/// Loads the attribute from disk into the memory storage.
	void Refresh();

	/// The size of data contained in the attribute.
	off_t Size();
	
	/// The data contained in the attribute.
	const void* Data();
	
private:
	const BNode *m_node;
	const char *m_attributeName;
	const type_code m_attributeType;
	BMallocIO *m_storage;
};

#endif

