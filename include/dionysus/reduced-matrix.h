#ifndef DIONYSUS_REDUCED_MATRIX_H
#define DIONYSUS_REDUCED_MATRIX_H

#include <vector>
#include <tuple>

#include "chain.h"
#include "reduction.h"

namespace dionysus
{

template<class Field_, typename Index_ = unsigned, class Comparison_ = std::less<Index_>, template<class Self> class... Visitors>
class ReducedMatrix
{
    public:
        typedef                 ReducedMatrix<Field_,Index_,Comparison_,Visitors...>    Self;

        typedef                 Field_                          Field;
        typedef                 Index_                          Index;
        typedef                 Comparison_                     Comparison;

        typedef                 std::tuple<Visitors<Self>...>   VisitorsTuple;
        template<size_t I>
        using Visitor = std::tuple_element<I, VisitorsTuple>;

        typedef                 typename Field::Element         FieldElement;
        typedef                 ChainEntry<Field, Index>        Entry;
        typedef                 std::vector<Entry>              Chain;

        typedef                 std::vector<Chain>              Chains;
        typedef                 std::vector<Index>              Indices;

    public:
                                ReducedMatrix(const Field&                field):
                                    field_(field)                               {}

                                ReducedMatrix(const Field&                field,
                                              const Comparison&           cmp,
                                              const Visitors<Self>&...    visitors):
                                    field_(field),
                                    cmp_(cmp),
                                    visitors_(visitors...)                      {}

                                ReducedMatrix(Field&&                     field,
                                              Comparison&&                cmp,
                                              Visitors<Self>&&...         visitors):
                                    field_(std::move(field)),
                                    cmp_(std::move(cmp)),
                                    visitors_(visitors...)                      {}

        template<class ChainRange>
        Index                   add(const ChainRange& chain);

        template<class ChainRange>
        Index                   set(Index i, const ChainRange& chain);

        Index                   reduce_upto(Index i);           // TODO

        const Chain&            operator[](Index i) const       { return reduced_[i]; }
        Index                   pair(Index i) const             { return pairs_[i]; }

        const Field&            field() const                   { return field_; }
        void                    reserve(size_t s)               { reduced_.reserve(s); pairs_.reserve(s); }
        void                    resize(size_t s)                { reduced_.resize(s); pairs_.resize(s, unpaired); }

        const Chains&           columns() const                 { return reduced_; }

        template<std::size_t I>
        Visitor<I>&             visitor()                       { return std::get<I>(visitors_); }

        static const Index      unpaired = Reduction<Index>::unpaired;

    public:
        // Visitors::chain_initialized(c)
        template<class Chain, std::size_t I = 0>
        typename std::enable_if<I == sizeof...(Visitors), void>::type
                                visitors_chain_initialized(Chain& c)        {}

        template<class Chain, std::size_t I = 0>
        typename std::enable_if<I < sizeof...(Visitors), void>::type
                                visitors_chain_initialized(Chain& c)        { std::get<I>(visitors_).chain_initialized(this, c); visitors_chain_initialized<Chain, I+1>(c); }

        // Visitors::addto(m, cl)
        template<std::size_t I = 0>
        typename std::enable_if<I == sizeof...(Visitors), void>::type
                                visitors_addto(FieldElement m, Index cl)    {}

        template<std::size_t I = 0>
        typename std::enable_if<I < sizeof...(Visitors), void>::type
                                visitors_addto(FieldElement m, Index cl)    { std::get<I>(visitors_).addto(this, m, cl); visitors_addto<I+1>(m, cl); }

        // Visitors::reduction_finished(m, cl)
        template<std::size_t I = 0>
        typename std::enable_if<I == sizeof...(Visitors), void>::type
                                visitors_reduction_finished()               {}

        template<std::size_t I = 0>
        typename std::enable_if<I < sizeof...(Visitors), void>::type
                                visitors_reduction_finished()               { std::get<I>(visitors_).reduction_finished(this); visitors_reduction_finished<I+1>(); }

    private:
        Field                   field_;
        Comparison              cmp_;
        Chains                  reduced_;       // matrix R
        Indices                 pairs_;
        VisitorsTuple           visitors_;
};

/*  Visitors */

// The prototypical visitor. Others may (and probably should) inherit from it.
template<class Field, typename Index, class Self = void>
struct EmptyVisitor
{
                EmptyVisitor() = default;

    template<class OtherSelf>
                EmptyVisitor(const EmptyVisitor<Field, Index, OtherSelf>&)  {}


    template<class Chain>
    void        chain_initialized(Self*, Chain& c)                          {}

    void        addto(Self*, typename Field::Element m, Index cl)           {}
    void        reduction_finished(Self*)                                   {}
};

}

#include "reduced-matrix.hpp"

#endif
