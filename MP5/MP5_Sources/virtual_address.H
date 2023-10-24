#ifndef _virtual_address_H_
#define _virtual_address_H_

class VirtualAddress
{
private:
	unsigned int _address;

public:
	VirtualAddress(unsigned int address) : _address(address) {}

	bool operator<(const VirtualAddress& other) const { return _address < other._address; }
	bool operator>(const VirtualAddress& other) const { return _address > other._address; }
	bool operator<=(const VirtualAddress& other) const { return _address <= other._address; }
	bool operator>=(const VirtualAddress& other) const { return _address >= other._address; }
	bool operator==(const VirtualAddress& other) const { return _address == other._address; }
	bool operator!=(const VirtualAddress& other) const { return _address != other._address; }
};

#endif // _virtual_address_H_
