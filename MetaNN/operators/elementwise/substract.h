#pragma once

#include <MetaNN/data/facilities/traits.h>
#include <MetaNN/evaluate/eval_plan.h>
#include <MetaNN/operators/facilities/operator_frame.h>
#include <stdexcept>

namespace MetaNN::OpTags
{
    struct Substract;
    struct SubstractFromNum;
}

namespace MetaNN
{
namespace OperSubstract::NSCaseGen
{
    template <typename TInputHandle1, typename TInputHandle2, typename TOutputHandle>
    class EvalItem : public BaseEvalItem<DeviceTypeFromHandle<TOutputHandle>>
    {
        using BaseType = BaseEvalItem<DeviceTypeFromHandle<TOutputHandle>>;
    public:
        template <typename TAuxParams>
        EvalItem(TInputHandle1 oriHandle1, TInputHandle2 oriHandle2, TOutputHandle outputHandle, const TAuxParams&)
            : BaseType(std::type_index(typeid(EvalItem)),
                       {oriHandle1.DataPtr(), oriHandle2.DataPtr()}, outputHandle.DataPtr())
            , m_inputHandle1(std::move(oriHandle1))
            , m_inputHandle2(std::move(oriHandle2))
            , m_outputHandle(std::move(outputHandle))
        {}
        
        const TInputHandle1 m_inputHandle1;
        const TInputHandle2 m_inputHandle2;
        TOutputHandle m_outputHandle;
    };

    template <typename TInputHandle1, typename TInputHandle2, typename TOutputHandle>
    class EvalGroup : public TrivalEvalGroup<EvalItem<TInputHandle1, TInputHandle2, TOutputHandle>>
    {
        using EvalItemType = EvalItem<TInputHandle1, TInputHandle2, TOutputHandle>;
    protected:
        virtual void EvalInternalLogic(EvalItemType& evalItem) final override
        {
            const auto& in1 = evalItem.m_inputHandle1.Data();
            const auto& in2 = evalItem.m_inputHandle2.Data();
            assert(in1.Shape() == in2.Shape());

            using ResType = typename TOutputHandle::DataType;
            using ElementType = typename ResType::ElementType;
            ResType out(in1.Shape());

            const size_t count = in1.Shape().Count();
            assert(count == out.Shape().Count());

            auto low_in1 = LowerAccess(in1);
            const ElementType* mem_in1 = low_in1.RawMemory();
            auto low_in2 = LowerAccess(in2);
            const ElementType* mem_in2 = low_in2.RawMemory();

            auto low_out = LowerAccess(out);
            ElementType* mem_out = low_out.MutableRawMemory();

            static_assert(std::is_same_v<DeviceTypeFromHandle<TOutputHandle>, DeviceTags::CPU>, "Currently only CPU is supported");

            for (size_t i = 0; i < count; ++i)
            {
                mem_out[i] = mem_in1[i] - mem_in2[i];
            }
            evalItem.m_outputHandle.SetData(std::move(out));
        }
    };
}

template <>
struct OperSeq_<OpTags::Substract>
{
    using type = OperCalAlgoChain<TailCalculator<OperSubstract::NSCaseGen::EvalItem, OperSubstract::NSCaseGen::EvalGroup>>;
};

/// Substract from number
namespace OperSubstractFromNum
{
template <typename TNumber, typename TOper>
constexpr bool Valid()
{
    if constexpr (IsOutOfDataCategory<TOper>)
    {
        return false;
    }
    else
    {
        return std::is_constructible_v<typename RemConstRef<TOper>::ElementType, TNumber>;
    }
}

namespace NSCaseGen
{
    template <typename TInputHandle, typename TOutputHandle>
    class EvalItem : public BaseEvalItem<DeviceTypeFromHandle<TOutputHandle>>
    {
        using BaseType = BaseEvalItem<DeviceTypeFromHandle<TOutputHandle>>;
    public:
        template <typename TAuxParams>
        EvalItem(TInputHandle oriHandle, TOutputHandle outputHandle, const TAuxParams& params)
            : BaseType(std::type_index(typeid(EvalItem)),
                       {oriHandle.DataPtr()}, outputHandle.DataPtr())
            , m_inputHandle(std::move(oriHandle))
            , m_minuend(params.Value())
            , m_outputHandle(std::move(outputHandle))
        {}
        
        const TInputHandle m_inputHandle;
        double m_minuend;
        TOutputHandle m_outputHandle;
    };

    template <typename TInputHandle, typename TOutputHandle>
    class EvalGroup : public TrivalEvalGroup<EvalItem<TInputHandle, TOutputHandle>>
    {
        using EvalItemType = EvalItem<TInputHandle, TOutputHandle>;
    protected:
        virtual void EvalInternalLogic(EvalItemType& evalItem) final override
        {
            const auto& input = evalItem.m_inputHandle.Data();

            using ResType = typename TOutputHandle::DataType;
            using ElementType = typename ResType::ElementType;
            ResType out(input.Shape());

            const size_t count = input.Shape().Count();
            assert(count == out.Shape().Count());

            auto low_in = LowerAccess(input);
            const ElementType* mem_in = low_in.RawMemory();

            auto low_out = LowerAccess(out);
            ElementType* mem_out = low_out.MutableRawMemory();

            static_assert(std::is_same_v<DeviceTypeFromHandle<TOutputHandle>, DeviceTags::CPU>, "Currently only CPU is supported");

            for (size_t i = 0; i < count; ++i)
            {
                mem_out[i] = evalItem.m_minuend - mem_in[i];
            }
            evalItem.m_outputHandle.SetData(std::move(out));
        }
    };
}}

template <typename TNumber, typename TOper>
constexpr bool IsValidOper<OpTags::SubstractFromNum, TNumber, TOper>
    = OperSubstractFromNum::Valid<TNumber, TOper>();

template <typename TCate>
struct OperAuxParams<OpTags::SubstractFromNum, TCate> : public OperAuxValue<double>
{
    using TBase = OperAuxValue<double>;
    using TBase::TBase;
    using TBase::operator =;
};

template <>
struct OperSeq_<OpTags::SubstractFromNum>
{
    using type = OperCalAlgoChain<TailCalculator<OperSubstractFromNum::NSCaseGen::EvalItem, OperSubstractFromNum::NSCaseGen::EvalGroup>>;
};

// Interface
template <typename TP1, typename TP2,
          typename = std::enable_if_t<IsValidOper<OpTags::Substract, TP1, TP2> ||
                                      IsValidOper<OpTags::SubstractFromNum, TP1, TP2>>>
auto operator- (TP1&& p_m1, TP2&& p_m2)
{
    if constexpr (IsValidOper<OpTags::Substract, TP1, TP2>)
    {
        if (p_m1.Shape() != p_m2.Shape())
        {
            throw std::runtime_error("Substract error: operands' shape mismatch.");
        }
    
        using rawOp1 = RemConstRef<TP1>;
        using rawOp2 = RemConstRef<TP2>;
        using ResType = Operator<OpTags::Substract, rawOp1, rawOp2>;
        return ResType(std::forward<TP1>(p_m1), std::forward<TP2>(p_m2));        
    }
    else if constexpr (IsValidOper<OpTags::SubstractFromNum, TP1, TP2>)
    {
        using rawOp = RemConstRef<TP2>;
        using ResType = Operator<OpTags::SubstractFromNum, rawOp>;
        OperAuxParams<OpTags::SubstractFromNum, OperCateCal<OpTags::SubstractFromNum, rawOp>> params(p_m1);
        return ResType(std::move(params), std::forward<TP2>(p_m2));
    }
    else
    {
        static_assert(DependencyFalse<TP1>);
    }
}
}