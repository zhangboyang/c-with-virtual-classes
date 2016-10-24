//////////////////// useful macros ////////////////////////////////////////////

#define _TOSTR(x) #x
#define TOSTR(x) _TOSTR(x)
#define _CONCAT(a, b) a ## b
#define CONCAT(a, b) _CONCAT(a, b)
#define CONCAT3(a, b, c) CONCAT(CONCAT(a, b), c)
#define CONCAT4(a, b, c, d) CONCAT(CONCAT3(a, b, c), d)
#define CONCAT5(a, b, c, d, e) CONCAT(CONCAT4(a, b, c, d), e)
#define CONCAT6(a, b, c, d, e, f) CONCAT(CONCAT5(a, b, c, d, e), f)

//////////////////// write virtual class in pure C ////////////////////////////
//////////////////// need  -fplan9-extensions /////////////////////////////////

#define THIS void *vthis
#define DECLARE_THIS(type) struct type *this = vthis

#define DECLARE_BASE_CLASS(base_class_name, base_class_vfunction, base_class_structure) \
    struct base_class_name { \
        struct CONCAT(base_class_name, __vtbl) { \
            base_class_vfunction; \
        } *vfptr; \
        struct CONCAT(base_class_name, __data) { \
            base_class_structure; \
        }; \
    }

#define DECLARE_DERIVED_CLASS(derived_class_name, base_class_name, derived_class_vfunction, derived_class_structure) \
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

#define MAKE_VTBL(class_name) struct CONCAT(class_name, __vtbl) CONCAT(class_name, __vtbl__instance)
#define INST_VTBL(class_name, ptr) (((ptr)->vfptr) = &CONCAT(class_name, __vtbl__instance))

#define INST_VTBL_SINGLETON(class_name, ptr, vtbl_data...) \
    do { \
        static MAKE_VTBL(class_name) = vtbl_data; \
        INST_VTBL(class_name, ptr); \
    } while (0) \

#define BASECLASS_CAST(ptr) (&(ptr)->baseclass)





//////////////////// sample test program //////////////////////////////////////

#include <stdio.h>

DECLARE_BASE_CLASS(class1, struct {
    // here are virtual functions
    void (*hello)(THIS);
}, struct {
    // here are member variables
    int a;
    int b;
});

DECLARE_DERIVED_CLASS(class2, class1, struct {
    // no new virtual functions
}, struct {
    int c;
});

DECLARE_DERIVED_CLASS(class3, class1, struct {
    void (*test)(THIS, int);
}, struct {
    // no new member variables
});

DECLARE_DERIVED_CLASS(class4, class3, struct {
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
    INST_VTBL_SINGLETON(class1, this, {
        .hello = class1_hello,
    });
    this->a = 100;
    this->b = 200;
}

void class2_ctor(struct class2 *this)
{
    INST_VTBL_SINGLETON(class2, this, {
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
    INST_VTBL_SINGLETON(class3, this, {
        .hello = class1_hello, // use base class virtual function
        .test = class3_test,
    });
    this->a = 111;
    this->b = 222;
}


int main()
{
    struct class1 c1;
    struct class2 c2;
    struct class3 c3;
    
    class1_ctor(&c1);
    class2_ctor(&c2);
    class3_ctor(&c3);
    
    (&c1)->vfptr->hello(&c1); // output line 1
    (&c2)->vfptr->hello(&c2); // output line 2
    (&c3)->vfptr->hello(&c3); // output line 3

    
    struct class1 *p1;
    struct class2 *p2;
    struct class3 *p3;

    p3 = &c3;
    p2 = &c2;
    p1 = BASECLASS_CAST(p2);
    
    p1->vfptr->hello(p1); // output line 4
    p2->vfptr->hello(p2); // output line 5
    p3->vfptr->test(p3, 12345); // output line 6
    
    
    return 0;
}
