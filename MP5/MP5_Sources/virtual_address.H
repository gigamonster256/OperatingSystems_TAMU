#ifndef _virtual_address_H_
#define _virtual_address_H_

class VirtualAddress
{
private:
	unsigned long _address;

public:
	VirtualAddress(unsigned int address) : _address(address) {}
	unsigned long address() const { return _address; }
	VirtualAddress offset(unsigned int offset) const { return VirtualAddress(_address + offset); }

	bool operator<(const VirtualAddress& other) const { return _address < other._address; }
	bool operator>(const VirtualAddress& other) const { return _address > other._address; }
	bool operator<=(const VirtualAddress& other) const { return _address <= other._address; }
	bool operator>=(const VirtualAddress& other) const { return _address >= other._address; }
	bool operator==(const VirtualAddress& other) const { return _address == other._address; }
	bool operator!=(const VirtualAddress& other) const { return _address != other._address; }
};

#endif // _virtual_address_H_
