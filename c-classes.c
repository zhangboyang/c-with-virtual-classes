// NOTE: compile using gcc with '-fplan9-extensions' enabled


//////////////////// useful macros ////////////////////////////////////////////

#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)
#define _CONCAT(a, b) a ## b
#define CONCAT(a, b) _CONCAT(a, b)
#define CONCAT3(a, b, c) CONCAT(CONCAT(a, b), c)
#define CONCAT4(a, b, c, d) CONCAT(CONCAT3(a, b, c), d)
#define CONCAT5(a, b, c, d, e) CONCAT(CONCAT4(a, b, c, d), e)
#define CONCAT6(a, b, c, d, e, f) CONCAT(CONCAT5(a, b, c, d, e), f)



//////////////////// class manipulate macros //////////////////////////////////

#define BASECLASS_CAST(ptr) (&(ptr)->baseclass)
#define THIS void *vthis
#define DECLARE_THIS(type) struct type *this = vthis



///////////// write derived class (without virtual funtcion) in pure C ////////

#define DECLARE_BASE_CLASS(base_class_name, base_class_structure...) \
    struct base_class_name { \
        base_class_structure; \
    }
#define DECLARE_DERIVED_CLASS(derived_class_name, base_class_name, derived_class_structure...) \
    struct derived_class_name { \
        union { \
            struct base_class_name; \
            struct base_class_name baseclass; \
        }; \
        derived_class_structure; \
    }



//////////////////// write virtual class in pure C ////////////////////////////

#define DECLARE_BASE_VCLASS(base_class_name, base_class_vfunction, base_class_structure...) \
    struct base_class_name { \
        struct CONCAT(base_class_name, __vtbl) { \
            base_class_vfunction; \
        } *vfptr; \
        struct CONCAT(base_class_name, __data) { \
            base_class_structure; \
        }; \
    }

#define DECLARE_DERIVED_VCLASS(derived_class_name, base_class_name, derived_class_vfunction, derived_class_structure...) \
    struct derived_class_name { \
        union { \
            struct base_class_name baseclass; \
            struct { \
                struct CONCAT(derived_class_name, __vtbl) { \
                    struct CONCAT(base_class_name, __vtbl); \
                    derived_class_vfunction; \
                } *vfptr; \
                struct CONCAT(derived_class_name, __data) { \
                    struct CONCAT(base_class_name, __data); \
                    derived_class_structure; \
                }; \
            }; \
        }; \
    }

#define INST_VTBL_SINGLETON(ptr, vtbl_data...) \
    do { \
        static typeof(*((ptr)->vfptr)) __c_with_virtual_class__vtbl_singleton = vtbl_data; \
        (ptr)->vfptr = &__c_with_virtual_class__vtbl_singleton; \
    } while (0) \

#define VFUNC(this, func, ...) ({ \
        typeof(this) __c_with_virtual_class__temp_this = (this); \
        __c_with_virtual_class__temp_this->vfptr->func(__c_with_virtual_class__temp_this, ##__VA_ARGS__); \
    })




//////////////////// sample test program //////////////////////////////////////

#include <stdio.h>

DECLARE_BASE_VCLASS(class1, struct {
    // here are virtual functions
    void (*hello)(THIS);
    
    // NOTE: you can't type comma without parenthesis in vfunc list
    //       for example, the following statment will cause an error
    //   void (*hello)(THIS), (*hello2)(THIS);
    //                      ^ no comma here!
}, struct {
    // here are member variables
    int a, b;
});

DECLARE_DERIVED_VCLASS(class2, class1, struct {
    // no new virtual functions
}, struct {
    int c;
});

DECLARE_DERIVED_VCLASS(class3, class1, struct {
    void (*test)(THIS, int);
}, struct {
    // no new member variables
});

DECLARE_DERIVED_VCLASS(class4, class3, struct {
    // no new virtual functions
}, struct {
    int d;
});


void class1_hello(THIS)
{
    DECLARE_THIS(class1);
    printf("hello1 a=%d b=%d\n", this->a, this->b);
}


void class2_hello(THIS)
{
    DECLARE_THIS(class2);
    printf("hello2 a=%d b=%d c=%d\n", this->a, this->b, this->c);
}

void class1_ctor(struct class1 *this)
{
    INST_VTBL_SINGLETON(this, {
        .hello = class1_hello,
    });
    this->a = 100;
    this->b = 200;
}

void class2_ctor(struct class2 *this)
{
    INST_VTBL_SINGLETON(this, {
        .hello = class2_hello,
    });
    this->a = 101;
    this->b = 202;
    this->c = 303;
}

void class3_test(THIS, int arg)
{
    //DECLARE_THIS(class3);
    printf("test arg=%d\n", arg);
}

void class3_ctor(struct class3 *this)
{
    INST_VTBL_SINGLETON(this, {
        .hello = class1_hello, // use base class virtual function
        .test = class3_test,
    });
    this->a = 111;
    this->b = 222;
}


DECLARE_BASE_CLASS(simple_class1, struct {
    int x, y, z;
});
DECLARE_DERIVED_CLASS(simple_class2, simple_class1, struct {
    int u, v, w;
});

void simple_class1_print(struct simple_class1 *this)
{
    printf("x=%d y=%d z=%d\n", this->x, this->y, this->z);
}


int main()
{
    struct class1 c1;
    struct class2 c2;
    struct class3 c3;
    
    class1_ctor(&c1);
    class2_ctor(&c2);
    class3_ctor(&c3);
    
    VFUNC(&c1, hello); // equals to (&c1)->vfptr->hello(&c1); // output line 1
    VFUNC(&c2, hello); // equals to (&c2)->vfptr->hello(&c2); // output line 2
    VFUNC(&c3, hello); // equals to (&c3)->vfptr->hello(&c3); // output line 3

    
    struct class1 *p1;
    struct class2 *p2;
    struct class3 *p3;

    p3 = &c3;
    p2 = &c2;
    p1 = BASECLASS_CAST(p2);
    
    VFUNC(p1, hello); // equals to p1->vfptr->hello(p1); // output line 4
    VFUNC(p2, hello); // equals to p2->vfptr->hello(p2); // output line 5
    VFUNC(p3, test, 12345); // equals to p3->vfptr->test(p3, 12345); // output line 6
    
    struct simple_class1 sc1;
    struct simple_class2 sc2;

    sc1.x = sc1.y = sc1.z = 12345;
    sc2.x = sc2.y = sc2.z = sc2.u = sc2.v = sc2.w = 23456;
    
    simple_class1_print(&sc1); // output line 7
    simple_class1_print(&sc2); // auto pointer cast // output line 8
    simple_class1_print(BASECLASS_CAST(&sc2)); // output line 9
    
    return 0;
}
