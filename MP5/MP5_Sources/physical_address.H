#ifndef _physical_address_H_
#define _physical_address_H_

class PhysicalAddress
{
private:
	unsigned int _address;

public:
	PhysicalAddress(unsigned int address) : _address(address) {}

	bool operator<(const PhysicalAddress& other) const { return _address < other._address; }
	bool operator>(const PhysicalAddress& other) const { return _address > other._address; }
	bool operator<=(const PhysicalAddress& other) const { return _address <= other._address; }
	bool operator>=(const PhysicalAddress& other) const { return _address >= other._address; }
	bool operator==(const PhysicalAddress& other) const { return _address == other._address; }
	bool operator!=(const PhysicalAddress& other) const { return _address != other._address; }
};



#endif // _physical_address_H_
