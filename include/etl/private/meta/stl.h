///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2026 BMW AG

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ETL_PRIVATE_META_STL_H_INCLUDED
#define ETL_PRIVATE_META_STL_H_INCLUDED

#define ETL_META_IMPL_STL

// The standard <meta> header does not yet provide reflect_invoke or
// is_access_specified.  The Bloomberg clang-p2996 <meta> does, via the
// __metafunction() compiler builtin.
#if defined(__clang__)
  #define ETL_HAS_META_REFLECT_INVOKE       1
  #define ETL_HAS_META_IS_ACCESS_SPECIFIED  1
  #define ETL_META_NEEDS_INFO_ARRAY         0
  // The Bloomberg clang-p2996 <meta> provides neither is_final (it only has the
  // is_final_type type trait) nor the data_member_options::bit_width member
  // (it is named 'width' there).
  #define ETL_HAS_META_IS_FINAL             0
  #define ETL_HAS_META_BIT_WIDTH_OPTION     0
#else
  #define ETL_HAS_META_REFLECT_INVOKE       0
  #define ETL_HAS_META_IS_ACCESS_SPECIFIED  0
  // GCC cannot keep a std::vector<info> alive in a constant expression, so
  // reflected member lists must be copied into an etl::meta::info_array first.
  #define ETL_META_NEEDS_INFO_ARRAY         1
  #define ETL_HAS_META_IS_FINAL             1
  #define ETL_HAS_META_BIT_WIDTH_OPTION     1
#endif

///\defgroup meta meta
///\ingroup etl

namespace etl
{
  namespace meta
  {
      //*************************************************************************
      // Standard <meta> path: delegate to std::meta (GCC 16+ / Bloomberg p2996).
      // This path is used when the standard <meta> header is available,
      // regardless of whether the full STL is enabled (ETL_USING_STL).
      //
      // Note: 'using namespace std::meta;' only enables unqualified lookup,
      // but user code calls etl::meta::foo (qualified lookup).  Explicit
      // using-declarations are required so that qualified names resolve.
      //
      // Some functions were added in later P2996 revisions and may not be
      // present in all implementations.  Guard those with __cpp_reflection
      // (defined by GCC 16+ but not by Bloomberg clang-p2996).
      //*************************************************************************

    using std::meta::info;
    using std::meta::member_offset;
    using std::meta::operators;

    // Import the operators enumerators (op_equals_equals, etc.)
    using enum std::meta::operators;

    // --- Naming & display --------------------------------------------------
    using std::meta::display_string_of;
    using std::meta::has_identifier;
    using std::meta::identifier_of;
    using std::meta::operator_of;
    using std::meta::source_location_of;
    using std::meta::symbol_of;
    using std::meta::u8display_string_of;
    using std::meta::u8identifier_of;
    using std::meta::u8symbol_of;

    // --- Accessors ---------------------------------------------------------
    using std::meta::constant_of;
    using std::meta::object_of;
    using std::meta::parent_of;
    using std::meta::type_of;
    #if defined(__cpp_reflection)
    using std::meta::has_parent;
    #endif
    using std::meta::dealias;
    using std::meta::has_template_arguments;
    using std::meta::template_arguments_of;
    using std::meta::template_of;
    #if ETL_HAS_PARAMETER_REFLECTION
    using std::meta::parameters_of;
    using std::meta::return_type_of;
    using std::meta::variable_of;
    #endif

    // --- Access predicates -------------------------------------------------
    using std::meta::is_accessible;
    using std::meta::is_private;
    using std::meta::is_protected;
    using std::meta::is_public;

    // --- Function / member qualifiers --------------------------------------
    using std::meta::is_override;
    using std::meta::is_pure_virtual;
    using std::meta::is_virtual;
    #if ETL_HAS_META_IS_FINAL
    using std::meta::is_final;
    #endif
    using std::meta::is_bit_field;
    using std::meta::is_const;
    using std::meta::is_defaulted;
    using std::meta::is_deleted;
    using std::meta::is_enumerator;
    using std::meta::is_explicit;
    using std::meta::is_lvalue_reference_qualified;
    using std::meta::is_mutable_member;
    using std::meta::is_noexcept;
    using std::meta::is_rvalue_reference_qualified;
    using std::meta::is_user_declared;
    using std::meta::is_user_provided;
    using std::meta::is_volatile;

    // --- Storage duration / linkage ----------------------------------------
    using std::meta::has_automatic_storage_duration;
    using std::meta::has_external_linkage;
    using std::meta::has_internal_linkage;
    using std::meta::has_module_linkage;
    using std::meta::has_static_storage_duration;
    using std::meta::has_thread_storage_duration;
    #if defined(__cpp_reflection)
    using std::meta::has_c_language_linkage;
    #endif
    using std::meta::has_linkage;

    // --- Entity kind predicates --------------------------------------------
    using std::meta::is_assignment;
    using std::meta::is_complete_type;
    using std::meta::is_constructor;
    using std::meta::is_conversion_function;
    using std::meta::is_copy_assignment;
    using std::meta::is_copy_constructor;
    using std::meta::is_default_constructor;
    using std::meta::is_destructor;
    using std::meta::is_enumerable_type;
    using std::meta::is_function;
    using std::meta::is_literal_operator;
    using std::meta::is_move_assignment;
    using std::meta::is_move_constructor;
    using std::meta::is_namespace;
    using std::meta::is_namespace_alias;
    using std::meta::is_operator_function;
    using std::meta::is_special_member_function;
    using std::meta::is_type;
    using std::meta::is_type_alias;
    using std::meta::is_variable;
    #if ETL_HAS_PARAMETER_REFLECTION
    using std::meta::has_default_argument;
    #if defined(__clang__)
    // Only provided by the Bloomberg clang-p2996 <meta>.
    using std::meta::has_ellipsis_parameter;
    #endif
    using std::meta::is_explicit_object_parameter;
    using std::meta::is_function_parameter;
    #endif // ETL_HAS_PARAMETER_REFLECTION

    // --- Template predicates -----------------------------------------------
    using std::meta::is_alias_template;
    using std::meta::is_class_template;
    using std::meta::is_concept;
    using std::meta::is_constructor_template;
    using std::meta::is_conversion_function_template;
    using std::meta::is_function_template;
    using std::meta::is_literal_operator_template;
    using std::meta::is_operator_function_template;
    using std::meta::is_template;
    using std::meta::is_variable_template;

    // --- Value / object / membership predicates ----------------------------
    using std::meta::has_default_member_initializer;
    using std::meta::is_base;
    using std::meta::is_class_member;
    using std::meta::is_namespace_member;
    using std::meta::is_nonstatic_data_member;
    using std::meta::is_object;
    using std::meta::is_static_member;
    using std::meta::is_structured_binding;
    using std::meta::is_value;

    // --- Compile-time value extraction / injection -------------------------
    using std::meta::reflect_constant;
    // using std::meta::extract;  // function template — imported via using-namespace
    using std::meta::reflect_object;

    // --- Member / base queries ---------------------------------------------
    using std::meta::bases_of;
    using std::meta::members_of;
    using std::meta::nonstatic_data_members_of;
    using std::meta::static_data_members_of;
    #if defined(__cpp_reflection)
    using std::meta::subobjects_of;
    #endif
    using std::meta::enumerators_of;
    #if ETL_HAS_ANNOTATION_ATTRIBUTES
    using std::meta::annotations_of;
    #endif

    // --- Layout queries ----------------------------------------------------
    using std::meta::alignment_of;
    using std::meta::bit_size_of;
    using std::meta::offset_of;
    using std::meta::size_of;

    // --- Type-trait mirrors ------------------------------------------------
    using std::meta::is_abstract_type;
    using std::meta::is_aggregate_type;
    using std::meta::is_assignable_type;
    using std::meta::is_bounded_array_type;
    using std::meta::is_copy_assignable_type;
    using std::meta::is_copy_constructible_type;
    using std::meta::is_default_constructible_type;
    using std::meta::is_destructible_type;
    using std::meta::is_empty_type;
    using std::meta::is_final_type;
    using std::meta::is_move_assignable_type;
    using std::meta::is_move_constructible_type;
    using std::meta::is_nothrow_assignable_type;
    using std::meta::is_nothrow_copy_assignable_type;
    using std::meta::is_nothrow_copy_constructible_type;
    using std::meta::is_nothrow_default_constructible_type;
    using std::meta::is_nothrow_destructible_type;
    using std::meta::is_nothrow_move_assignable_type;
    using std::meta::is_nothrow_move_constructible_type;
    using std::meta::is_nothrow_swappable_type;
    using std::meta::is_nothrow_swappable_with_type;
    using std::meta::is_polymorphic_type;
    using std::meta::is_scoped_enum_type;
    using std::meta::is_signed_type;
    using std::meta::is_swappable_type;
    using std::meta::is_swappable_with_type;
    using std::meta::is_trivially_assignable_type;
    using std::meta::is_trivially_copy_assignable_type;
    using std::meta::is_trivially_copy_constructible_type;
    using std::meta::is_trivially_default_constructible_type;
    using std::meta::is_trivially_destructible_type;
    using std::meta::is_trivially_move_assignable_type;
    using std::meta::is_trivially_move_constructible_type;
    using std::meta::is_unbounded_array_type;
    using std::meta::is_unsigned_type;
    #if defined(__cpp_reflection)
    using std::meta::is_implicit_lifetime_type;
    #endif
    using std::meta::has_unique_object_representations;
    using std::meta::has_virtual_destructor;
    using std::meta::is_base_of_type;
    using std::meta::is_convertible_type;
    using std::meta::is_nothrow_convertible_type;
    using std::meta::is_same_type;
    #if defined(__cpp_reflection)
    using std::meta::is_layout_compatible_type;
    #endif
    using std::meta::extent;
    using std::meta::rank;

    // --- Type transformations ----------------------------------------------
    using std::meta::add_const;
    using std::meta::add_cv;
    using std::meta::add_lvalue_reference;
    using std::meta::add_pointer;
    using std::meta::add_rvalue_reference;
    using std::meta::add_volatile;
    using std::meta::decay;
    using std::meta::make_signed;
    using std::meta::make_unsigned;
    using std::meta::remove_all_extents;
    using std::meta::remove_const;
    using std::meta::remove_cv;
    using std::meta::remove_cvref;
    using std::meta::remove_extent;
    using std::meta::remove_pointer;
    using std::meta::remove_reference;
    using std::meta::remove_volatile;
    using std::meta::underlying_type;
    using std::meta::unwrap_ref_decay;
    using std::meta::unwrap_reference;
    #if defined(__cpp_reflection)
    using std::meta::type_order;
    #endif

    // --- Splice helpers (define_aggregate, data_member_spec, etc.) ---------
    using std::meta::data_member_spec;
    using std::meta::define_aggregate;
    using std::meta::is_data_member_spec;

    // --- extract is a function template; keep using-namespace for templates
    using namespace std::meta;

    // Bloomberg's <meta> may not provide is_structural_type; implement it
    // using the compiler builtin if it wasn't pulled in by the using-namespace.
    // The __metafunction() builtin only exists in the Bloomberg clang-p2996 fork,
    // so other implementations (e.g. GCC 16) must use the standard declaration.
    #if defined(__clang__)
    namespace detail
    {
      enum : unsigned
      {
        __metafn_is_structural_type_std = 6
      };
    }

    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wlanguage-extension-token\"")
      consteval auto is_structural_type(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_structural_type_std, r);
    }
    _Pragma("clang diagnostic pop")
    #else
    using std::meta::is_structural_type;
    #endif

    //*************************************************************************
    /// info_array — fixed-size, zero-allocation array of info values.
    ///
    /// GCC limitation: template-for over a std::vector<info> fails because the
    /// vector's heap allocation cannot persist in a constant expression.  The
    /// workaround is to copy the elements into a plain aggregate array (no
    /// allocation) and iterate that instead.
    ///
    /// Usage pattern:
    ///   constexpr auto N   = std::meta::members_of(^^T, ctx).size();
    ///   constexpr auto arr = etl::meta::to_info_array<N>(
    ///                            std::meta::members_of(^^T, ctx));
    ///   template for (constexpr auto m : arr) { ... }
    //*************************************************************************
    template <size_t N>
    struct info_array
    {
      info elems[N > 0 ? N : 1]{};
      consteval const info* begin() const { return elems; }
      consteval const info* end()   const { return elems + N; }
      consteval size_t      size()  const { return N; }
      consteval const info& operator[](size_t i) const { return elems[i]; }
    };

    template <size_t N>
    consteval info_array<N> to_info_array(const std::vector<info>& v)
    {
      info_array<N> a{};
      for (size_t i = 0; i < N; ++i)
      {
        a.elems[i] = v[i];
      }
      return a;
    }


  } // namespace meta
} // namespace etl

#endif // ETL_PRIVATE_META_STL_H_INCLUDED
