#include "med/Maps.hpp"
#include "med/MedException.hpp"

const AddressPair& Maps::operator[](size_t index) const {
    if (index >= maps_.size()) {
        throw MedException("Maps index out of range");
    }
    return maps_[index];
}

AddressPair& Maps::operator[](size_t index) {
    if (index >= maps_.size()) {
        throw MedException("Maps index out of range");
    }
    return maps_[index];
}

const AddressPairs& Maps::getMaps() const {
    return maps_;
}

AddressPairs& Maps::getMaps() {
    return maps_;
}

bool Maps::hasPair(const AddressPair& pair) const {
    for (const auto& p : maps_) {
        if (p == pair) return true;
    }
    return false;
}

void Maps::push(const AddressPair& pair) {
    maps_.push_back(pair);
}

void Maps::trimByScope(const AddressPair& scope) {
    AddressPairs newMaps;
    for (const auto& p : maps_) {
        Address start = std::max(p.first, scope.first);
        Address end = p.second;
        if (scope.second > 0) {
            end = std::min(p.second, scope.second);
        }
        if (start < end) {
            newMaps.emplace_back(start, end);
        }
    }
    maps_ = std::move(newMaps);
}

size_t Maps::size() const {
    return maps_.size();
}

void Maps::clear() {
    maps_.clear();
}
