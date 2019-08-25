/**
 * CacheReplacement.cpp
 * Cache replacement algorithms module.
 * @author Oleg Ladin, Denis Los, Andrey Agrachev
 */

#include "infra/replacement/cache_replacement.h"
#include "infra/macro.h"

#include <sparsehash/dense_hash_map.h>

#include <list>

class LRU : public CacheReplacement
{
    public:
        explicit LRU( std::size_t ways);

        void touch( std::size_t way) override ;
        void set_to_erase( std::size_t way) override ;
        std::size_t update() override ;
        std::size_t get_ways() const override { return ways; }

    private:
        std::list<std::size_t> lru_list{};
        const std::size_t ways;
        const std::size_t impossible_key = SIZE_MAX;
        google::dense_hash_map<std::size_t, decltype(lru_list.cbegin())> lru_hash{};
};

LRU::LRU( std::size_t ways) : ways( ways), lru_hash( ways)
{
    lru_hash.set_empty_key( impossible_key); //special dense_hash_map requirement
    for ( std::size_t i = 0; i < ways; i++)
    {
        lru_list.push_front( i);
        lru_hash.emplace( i, lru_list.begin());
    }
}

void LRU::touch( std::size_t way)
{
    const auto lru_it = lru_hash.find( way);
    assert( lru_it != lru_hash.end());
    // Put the way to the head of the list
    lru_list.splice( lru_list.begin(), lru_list, lru_it->second);
}

void LRU::set_to_erase( std::size_t way)
{
    const auto lru_it = lru_hash.find( way);
    assert( lru_it != lru_hash.end());
    lru_list.splice( lru_list.end(), lru_list, lru_it->second);
}

std::size_t LRU::update()
{
    // remove the least recently used element from the tail
    std::size_t lru_elem = lru_list.back();
    lru_list.pop_back();

    // put it to the head
    auto ptr = lru_list.insert( lru_list.begin(), lru_elem);
    lru_hash[ lru_elem] = ptr;

    return lru_elem;
}

//////////////////////////////////////////////////////////////////

class PseudoLRU : public CacheReplacement
{
    public:
        explicit PseudoLRU( std::size_t ways);
        void touch( std::size_t way) override;
        void set_to_erase( std::size_t /* unused */) override;
        std::size_t update() override;
        std::size_t get_ways() const override { return ways; }

    private:
        enum Flags { Left = 0, Right = 1};

        /*
         *    0
         *   / \ 
         *  1   2  - nodes
         * / \ / \
         * 3 4 5 6 - leaves
         * 0 1 2 3 - ways
         */
        size_t get_next_node( size_t node) const { return node * 2 + ( nodes[node] == Left ? 1 : 2); }
        static Flags get_direction_to_prev_node( size_t node) { return node % 2 != 0 ? Left : Right; }
        void reverse_node( size_t node) { nodes[node] = ( nodes[node] == Left ? Right : Left); }

        std::vector<Flags> nodes;
        const std::size_t ways;
};

PseudoLRU::PseudoLRU( std::size_t ways) : nodes( ways - 1, Left), ways( ways)
{
    if (!is_power_of_two( ways))
        throw CacheReplacementException("Number of ways must be the power of 2!");
}

void PseudoLRU::touch( std::size_t way)
{
    auto node = way + nodes.size();
    while ( node != 0) {
        const auto parent = ( node - 1) / 2;
        if ( get_direction_to_prev_node( node) == nodes[parent])
            reverse_node( parent);
        node = parent;
    }
}

std::size_t PseudoLRU::update()
{
    size_t node = 0;
    while ( node < nodes.size())
        node = get_next_node(node);

    const auto way = node - nodes.size();
    touch( way);
    return way;
}

void PseudoLRU::set_to_erase( std::size_t /* way */)
{
    throw CacheReplacementException( "PLRU does not support inverted access");
}

////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<CacheReplacement> create_cache_replacement( const std::string& name, std::size_t ways)
{
    if (name == "LRU")
        return std::make_unique<LRU>( ways);

    if (name == "Pseudo-LRU")
        return std::make_unique<PseudoLRU>( ways);

    throw CacheReplacementException("\"" + name + "\" replacement policy is not defined, supported polices are:\nLRU\npseudo-LRU\n");
}
