#ifndef MED_MAPS_HPP
#define MED_MAPS_HPP

#include <vector>
#include "med/MedTypes.hpp"

class Maps {
public:
    Maps() = default;

    const AddressPair& operator[](size_t index) const;
    AddressPair& operator[](size_t index);

    const AddressPairs& getMaps() const;
    AddressPairs& getMaps();

    bool hasPair(const AddressPair& pair) const;
    void push(const AddressPair& pair);
    void trimByScope(const AddressPair& scope);
    size_t size() const;
    void clear();

private:
    AddressPairs maps_;
};

#endif
