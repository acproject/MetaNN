#pragma once

#include <MetaNN/layers/facilities/common_io.h>
#include <MetaNN/layers/facilities/policies.h>
#include <MetaNN/layers/facilities/traits.h>
#include <MetaNN/policies/policy_operations.h>
#include <MetaNN/policies/policy_selector.h>
#include <stack>

namespace MetaNN
{
    template <typename TInputs, typename TGrads, typename TPolicies>
    class ReLULayer
    {
        static_assert(IsPolicyContainer<TPolicies>);
        using CurLayerPolicy = PlainPolicy<TPolicies>;

    public:
        static constexpr bool IsFeedbackOutput = PolicySelect<GradPolicy, CurLayerPolicy>::IsFeedbackOutput;
        static constexpr bool IsUpdate = false;

        using InputMap = TInputs;
        using GradMap = TGrads;
        
    private:
        using TLayerInputFP = typename InputMap::template Find<LayerInput>;
        using TLayerOutputBP = typename GradMap::template Find<LayerOutput>;

    public:
        ReLULayer(std::string name)
            : m_name(std::move(name))
        {}
        
        template <typename TIn>
        auto FeedForward(TIn&& p_in)
        {
            auto val = LayerTraits::PickItemFromCont<InputMap, LayerInput>(std::forward<TIn>(p_in));
            auto res = ReLU(val);

            if constexpr (IsFeedbackOutput)
            {
                m_inputShape.PushDataShape(val);
                m_outputShape.PushDataShape(res);
                m_data.push(std::move(val));
            }
            return LayerOutputCont<ReLULayer>().template Set<LayerOutput>(std::move(res));
        }

        template <typename TGrad>
        auto FeedBackward(TGrad&& p_grad)
        {
            if constexpr (IsFeedbackOutput)
            {
                if (m_data.empty())
                {
                    throw std::runtime_error("Cannot feed back in ReLULayer");
                }
                auto grad = LayerTraits::PickItemFromCont<GradMap, LayerOutput>(std::forward<TGrad>(p_grad));
                m_outputShape.CheckDataShapeAndPop(grad);
                
                auto input = m_data.top();
                m_data.pop();
                
                auto res = ReLUGrad(std::move(grad), std::move(input));
                m_inputShape.CheckDataShapeAndPop(res);
                return LayerInputCont<ReLULayer>().template Set<LayerInput>(std::move(res));
            }
            else
            {
                return LayerInputCont<ReLULayer>();
            }
        }

        void NeutralInvariant() const
        {
            if constexpr(IsFeedbackOutput)
            {
                if (!m_data.empty())
                {
                    throw std::runtime_error("NeutralInvariant Fail!");
                }
                m_inputShape.AssertEmpty();
                m_outputShape.AssertEmpty();
            }
        }

    private:
        std::string m_name;
        LayerTraits::LayerInternalBuf<TLayerInputFP, IsFeedbackOutput> m_data;

        LayerTraits::ShapeChecker<TLayerInputFP,  IsFeedbackOutput> m_inputShape;
        LayerTraits::ShapeChecker<TLayerOutputBP, IsFeedbackOutput> m_outputShape;
    };
}