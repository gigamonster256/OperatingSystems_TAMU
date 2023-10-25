#ifndef _physical_address_H_
#define _physical_address_H_

class PhysicalAddress
{
private:
	unsigned long _address;

public:
	PhysicalAddress(unsigned int address) : _address(address) {}
	unsigned long address() const { return _address; }
	unsigned long page() const { return _address >> 12; }
	unsigned long offset() const { return _address & 0xFFF; }

	bool operator<(const PhysicalAddress& other) const { return _address < other._address; }
	bool operator>(const PhysicalAddress& other) const { return _address > other._address; }
	bool operator<=(const PhysicalAddress& other) const { return _address <= other._address; }
	bool operator>=(const PhysicalAddress& other) const { return _address >= other._address; }
	bool operator==(const PhysicalAddress& other) const { return _address == other._address; }
	bool operator!=(const PhysicalAddress& other) const { return _address != other._address; }
};



#endif // _physical_address_H_
