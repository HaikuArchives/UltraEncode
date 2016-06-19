

#include <StorageKit.h>
#include <Node.h>
#include <fs_attr.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "DebugBuild.h"
#include "Attribute.h"


Attribute::Attribute(const BNode *node, const char *name, bool createNew):
	m_node(node), m_attributeType(B_RAW_TYPE) {
	m_storage = new BMallocIO();
	m_storage->SetBlockSize(4096);

	m_attributeName = strdup(name);
	
	if (!createNew)
		Refresh();
}

Attribute::~Attribute() {
	Flush();
	free((char*)m_attributeName);
}

ssize_t Attribute::Read(void *buffer, size_t size) {
	return m_storage->Read(buffer, size);
}

ssize_t Attribute::Write(const void *buffer, size_t size) {
	return m_storage->Write(buffer, size);
}

ssize_t Attribute::ReadAt(off_t pos, void *buffer, size_t size) {
	return m_storage->ReadAt(pos, buffer, size);
}

ssize_t Attribute::WriteAt(off_t pos, const void *buffer, size_t size) {
	return m_storage->WriteAt(pos, buffer, size);
}


off_t Attribute::Seek(off_t position, uint32 seek_mode) {
	return m_storage->Seek(position, seek_mode);
}

off_t Attribute::Position() const {
	return m_storage->Position();
}

off_t Attribute::Size() {
	return m_storage->BufferLength();
}

const void* Attribute::Data() {
	return m_storage->Buffer();
}

status_t Attribute::SetSize(off_t size) {
	return m_storage->SetSize(size);
}


void Attribute::Flush() {
	printf("upon flushing, buffer size=%ld\n",m_storage->BufferLength());
	((BNode*)m_node)->WriteAttr(m_attributeName, m_attributeType, 0,
			m_storage->Buffer(), m_storage->BufferLength());
}

void Attribute::Refresh() 
{
	attr_info info;
	m_node->GetAttrInfo(m_attributeName, &info);
		
	m_storage->SetSize(info.size);
	assert(m_storage->Buffer() != NULL);
	
	status_t read = m_node->ReadAttr(m_attributeName, m_attributeType, 0,
		(void*)m_storage->Buffer(), m_storage->BufferLength());
	printf("attr %s size=%Ld read=%ld\n",m_attributeName, info.size, read);
}

