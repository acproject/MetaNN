#include <data_gen.h>
#include <MetaNN/meta_nn.h>
#include <calculate_tags.h>
#include <cmath>
#include <iostream>
using namespace std;
using namespace MetaNN;

namespace
{
    void test_sigmoid_case1()
    {
        cout << "Test sigmoid case 1 (scalar)\t";
        {
            Scalar<CheckElement, CheckDevice> ori(0.9213);
            auto op = Sigmoid(ori);
            auto res = Evaluate(op);
            assert(fabs(res.Value() - 0.7153) < 0.001f);
        }
        cout << "done" << endl;
    }
    
    void test_sigmoid_case2()
    {
        cout << "Test sigmoid case 2 (matrix)\t";
        auto ori = GenMatrix<CheckElement>(10, 7, -1, 0.01);
        auto op = Sigmoid(ori);
        static_assert(IsMatrix<decltype(op)>);
        assert(op.Shape().RowNum() == 10);
        assert(op.Shape().ColNum() == 7);
        
        auto res = Evaluate(op);
        static_assert(IsMatrix<decltype(res)>);
        assert(res.Shape().RowNum() == 10);
        assert(res.Shape().ColNum() == 7);
        
        for (size_t i = 0; i < 10; ++i)
        {
            for (size_t k = 0; k < 7; ++k)
            {
                auto value = 1 / (1 + exp(-ori(i, k)));
                assert(fabs(res(i, k) - value) < 0.001f);
            }
        }
        cout << "done" << endl;
    }
    
    void test_sigmoid_case3()
    {
        cout << "Test sigmoid case 3 (3d-array)\t";
        auto ori = GenThreeDArray<CheckElement>(2, 10, 7, -1, 0.01);
        auto op = Sigmoid(ori);
        static_assert(IsThreeDArray<decltype(op)>);
        assert(op.Shape().PageNum() == 2);
        assert(op.Shape().RowNum() == 10);
        assert(op.Shape().ColNum() == 7);
        
        auto res = Evaluate(op);
        static_assert(IsThreeDArray<decltype(res)>);
        assert(res.Shape().PageNum() == 2);
        assert(res.Shape().RowNum() == 10);
        assert(res.Shape().ColNum() == 7);
        
        for (size_t p = 0; p < 2; ++p)
        {
            for (size_t i = 0; i < 10; ++i)
            {
                for (size_t k = 0; k < 7; ++k)
                {
                    auto value = 1 / (1 + exp(-ori(p, i, k)));
                    assert(fabs(res(p, i, k) - value) < 0.001f);
                }
            }
        }
        cout << "done" << endl;
    }
    
    void test_sigmoid_case4()
    {
        cout << "Test sigmoid case 4 (batch scalar)\t";
        auto ori = GenBatchScalar<CheckElement>(10, -1, 0.1);
        auto op = Sigmoid(ori);
        static_assert(IsBatchScalar<decltype(op)>);
        assert(op.Shape().BatchNum() == 10);
        
        auto res = Evaluate(op);
        static_assert(IsBatchScalar<decltype(res)>);
        assert(res.Shape().BatchNum() == 10);
        
        for (size_t i = 0; i < 10; ++i)
        {
            auto value = 1 / (1 + exp(-ori[i].Value()));
            assert(fabs(res[i].Value() - value) < 0.001f);
        }
        cout << "done" << endl;
    }
    
    void test_sigmoid_case5()
    {
        cout << "Test sigmoid case 5 (batch matrix)\t";
        auto ori = GenBatchMatrix<CheckElement>(2, 10, 7, -0.9, 0.01);
        auto op = Sigmoid(ori);
        static_assert(IsBatchMatrix<decltype(op)>);
        assert(op.Shape().BatchNum() == 2);
        assert(op.Shape().RowNum() == 10);
        assert(op.Shape().ColNum() == 7);
        
        auto res = Evaluate(op);
        static_assert(IsBatchMatrix<decltype(res)>);
        assert(res.Shape().BatchNum() == 2);
        assert(res.Shape().RowNum() == 10);
        assert(res.Shape().ColNum() == 7);
        
        for (size_t p = 0; p < 2; ++p)
        {
            for (size_t i = 0; i < 10; ++i)
            {
                for (size_t k = 0; k < 7; ++k)
                {
                    auto value = 1 / (1 + exp(-ori[p](i, k)));
                    assert(fabs(res[p](i, k) - value) < 0.001f);
                }
            }
        }
        cout << "done" << endl;
    }
    
    void test_sigmoid_case6()
    {
        cout << "Test sigmoid case 6 (batch 3d-array)\t";
        auto ori = GenBatchThreeDArray<CheckElement>(2, 3, 10, 7);
        auto op = Sigmoid(ori);
        static_assert(IsBatchThreeDArray<decltype(op)>);
        assert(op.Shape().BatchNum() == 2);
        assert(op.Shape().PageNum() == 3);
        assert(op.Shape().RowNum() == 10);
        assert(op.Shape().ColNum() == 7);
        
        auto res = Evaluate(op);
        static_assert(IsBatchThreeDArray<decltype(res)>);
        assert(res.Shape().BatchNum() == 2);
        assert(res.Shape().PageNum() == 3);
        assert(res.Shape().RowNum() == 10);
        assert(res.Shape().ColNum() == 7);
        
        for (size_t b = 0; b < 2; ++b)
        {
            for (size_t p = 0; p < 3; ++p)
            {
                for (size_t i = 0; i < 10; ++i)
                {
                    for (size_t k = 0; k < 7; ++k)
                    {
                        auto value = 1 / (1 + exp(-ori[b](p, i, k)));
                        assert(fabs(res[b](p, i, k) - value) < 0.001f);
                    }
                }
            }
        }
        cout << "done" << endl;
    }
}

namespace Test::Operators::Activation
{
    void test_sigmoid()
    {
        test_sigmoid_case1();
        test_sigmoid_case2();
        test_sigmoid_case3();
        test_sigmoid_case4();
        test_sigmoid_case5();
        test_sigmoid_case6();
    }
}


namespace
{
    void test_sigmoid_grad_case1()
    {
        cout << "Test sigmoid grad case 1 (scalar)\t";
        Scalar<CheckElement, CheckDevice> grad(3);
        Scalar<CheckElement, CheckDevice> inpu(9);
        auto op = SigmoidGrad(grad, inpu);
        static_assert(IsScalar<decltype(op)>);
        
        auto res = Evaluate(op);
        static_assert(IsScalar<decltype(res)>);
        auto value = 3 * 9 * (1 - 9);
        assert(fabs(res.Value() - value) < 0.001f);
        cout << "done" << endl;
    }
    
    void test_sigmoid_grad_case2()
    {
        cout << "Test sigmoid grad case 2 (matrix)\t";
        auto grad = GenMatrix<CheckElement>(10, 7, -100, 3);
        auto inpu = GenMatrix<CheckElement>(10, 7, 1, 1.5);
        auto op = SigmoidGrad(grad, inpu);
        static_assert(IsMatrix<decltype(op)>);
        assert(op.Shape().RowNum() == 10);
        assert(op.Shape().ColNum() == 7);
        
        auto res = Evaluate(op);
        static_assert(IsMatrix<decltype(res)>);
        assert(res.Shape().RowNum() == 10);
        assert(res.Shape().ColNum() == 7);
        
        for (size_t i = 0; i < 10; ++i)
        {
            for (size_t k = 0; k < 7; ++k)
            {
                auto value = grad(i, k) * inpu(i, k) * (1 - inpu(i, k));
                assert(fabs(value - res(i, k)) < 0.001f);
            }
        }
        cout << "done" << endl;
    }
}

namespace Test::Operators::Activation
{
    void test_sigmoid_grad()
    {
        test_sigmoid_grad_case1();
        test_sigmoid_grad_case2();
    }
}