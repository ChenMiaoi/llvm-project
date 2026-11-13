// RUN: %clang_cc1 -fsyntax-only -std=c++11 -Wunused-parameter -DEXPECT_NORMAL -DEXPECT_VIRTUAL -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++11 -Wunused-parameter -Wno-unused-virtual-parameter -DEXPECT_NORMAL -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++11 -Wno-unused-parameter -Wunused-virtual-parameter -DEXPECT_VIRTUAL -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++11 -Wunused-parameter -Werror=unused-virtual-parameter -DEXPECT_NORMAL -DEXPECT_VIRTUAL_ERROR -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++11 -Wno-unused-parameter -verify %s

struct VirtualBase {
  virtual void f(int unused) {} // #base
  virtual void outOfLine(int unused);
  void nonVirtual(int unused) {} // #non_virtual

  virtual void used(int value) { (void)value; }
  virtual void annotated(int unused __attribute__((unused))) {}
  virtual void unnamed(int) {}

  // A lambda inside a virtual method is still a non-virtual function.
  virtual void nested() {
    [](int unused) {}(0); // #lambda
  }
};

void VirtualBase::outOfLine(int unused) {} // #out_of_line

struct ExplicitOverride : VirtualBase {
  void f(int unused) override {} // #explicit_override
};

struct ImplicitOverride : VirtualBase {
  void f(int unused) {} // #implicit_override
};

// Virtualness is inherited through multiple levels without override specifiers.
struct FurtherOverride : ImplicitOverride {
  void f(int unused) {} // #further_override
  void nonVirtual(int unused) {} // #derived_non_virtual
};

template <class T> struct VirtualTemplate {
  virtual void f(T unused) {} // #virtual_template
};
VirtualTemplate<int> virtualTemplate;

// An override specifier identifies a virtual function even before its
// dependent base is resolved, including in an out-of-line definition.
template <class Base> struct DependentOverride : Base {
  void f(int unused) override;
};
template <class Base> void DependentOverride<Base>::f(int unused) {} // #dependent_override
DependentOverride<VirtualBase> dependentOverride;

template <class Base> struct DependentFinal : Base {
  void f(int unused) final {} // #dependent_final
};
DependentFinal<VirtualBase> dependentFinal;

// Without a virtual/override/final specifier, virtualness is not known at the
// point where unused parameters in the template definition are diagnosed.
template <class Base> struct DependentImplicit : Base {
  void f(int unused) {} // #dependent_implicit
};
DependentImplicit<VirtualBase> dependentImplicit;

#ifdef EXPECT_NORMAL
// expected-warning@#non_virtual {{unused parameter 'unused'}}
// expected-warning@#lambda {{unused parameter 'unused'}}
// expected-warning@#derived_non_virtual {{unused parameter 'unused'}}
// expected-warning@#dependent_implicit {{unused parameter 'unused'}}
#endif

#ifdef EXPECT_VIRTUAL
// expected-warning@#base {{unused parameter 'unused' in virtual function 'VirtualBase::f'}}
// expected-warning@#out_of_line {{unused parameter 'unused' in virtual function 'VirtualBase::outOfLine'}}
// expected-warning@#explicit_override {{unused parameter 'unused' in virtual function 'ExplicitOverride::f'}}
// expected-warning@#implicit_override {{unused parameter 'unused' in virtual function 'ImplicitOverride::f'}}
// expected-warning@#further_override {{unused parameter 'unused' in virtual function 'FurtherOverride::f'}}
// expected-warning@#virtual_template {{unused parameter 'unused' in virtual function 'VirtualTemplate::f'}}
// expected-warning@#dependent_override {{unused parameter 'unused' in virtual function 'DependentOverride::f'}}
// expected-warning@#dependent_final {{unused parameter 'unused' in virtual function 'DependentFinal::f'}}
#endif

#ifdef EXPECT_VIRTUAL_ERROR
// expected-error@#base {{unused parameter 'unused' in virtual function 'VirtualBase::f'}}
// expected-error@#out_of_line {{unused parameter 'unused' in virtual function 'VirtualBase::outOfLine'}}
// expected-error@#explicit_override {{unused parameter 'unused' in virtual function 'ExplicitOverride::f'}}
// expected-error@#implicit_override {{unused parameter 'unused' in virtual function 'ImplicitOverride::f'}}
// expected-error@#further_override {{unused parameter 'unused' in virtual function 'FurtherOverride::f'}}
// expected-error@#virtual_template {{unused parameter 'unused' in virtual function 'VirtualTemplate::f'}}
// expected-error@#dependent_override {{unused parameter 'unused' in virtual function 'DependentOverride::f'}}
// expected-error@#dependent_final {{unused parameter 'unused' in virtual function 'DependentFinal::f'}}
#endif

#if !defined(EXPECT_NORMAL) && !defined(EXPECT_VIRTUAL) && \
    !defined(EXPECT_VIRTUAL_ERROR)
// expected-no-diagnostics
#endif
