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

#ifndef ETL_PRIVATE_META_CLANG_P2996_H_INCLUDED
#define ETL_PRIVATE_META_CLANG_P2996_H_INCLUDED

#define ETL_META_IMPL_CLANG_P2996

#define ETL_HAS_META_REFLECT_INVOKE       1
#define ETL_HAS_META_IS_ACCESS_SPECIFIED  1
#define ETL_META_NEEDS_INFO_ARRAY         0
// Only is_final_type is provided, and data_member_options uses 'width'.
#define ETL_HAS_META_IS_FINAL             0
#define ETL_HAS_META_BIT_WIDTH_OPTION     0

///\defgroup meta meta
///\ingroup etl

namespace etl
{
  namespace meta
  {

        // for every use of this builtin.  Suppress the warning in this section
        // since the extension is integral to this implementation path.
    #if defined(__clang__)
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wlanguage-extension-token\"")
    #endif

      //*************************************************************************
      /// An opaque handle to a compile-time reflected entity.
      //*************************************************************************
      using info = decltype(^^int);

    //*************************************************************************
    /// A range whose elements are meta::info values.
    //*************************************************************************
    template <typename R>
    concept reflection_range = requires(R& r) {
      { *etl::ranges::begin(r) } -> etl::same_as<info>;
    } && etl::is_same_v<etl::remove_cvref_t<etl::ranges::range_value_t<R> >, info>;

    //*************************************************************************
    /// detail: Compiler metafunction IDs for __metafunction() builtin.
    /// These must match the IDs used by the compiler (Bloomberg clang-p2996).
    //*************************************************************************
    namespace detail
    {
      enum : unsigned
      {
        // Internal iteration helpers (non-exposed)
        __metafn_get_begin_enumerator_decl_of,
        __metafn_get_get_next_enumerator_decl_of,
        __metafn_get_ith_base_of,
        __metafn_get_ith_template_argument_of,
        __metafn_get_begin_member_decl_of,
        __metafn_get_next_member_decl_of,
        __metafn_is_structural_type,
        __metafn_map_decl_to_entity,

        // P2996 metafunctions
        __metafn_identifier_of,
        __metafn_has_identifier,
        __metafn_operator_of,
        __metafn_source_location_of,
        __metafn_type_of,
        __metafn_parent_of,
        __metafn_underlying_entity_of,
        __metafn_proxied_entity_of,
        __metafn_object_of,
        __metafn_constant_of,
        __metafn_template_of,
        __metafn_substitute,
        __metafn_extract,
        __metafn_is_public,
        __metafn_is_protected,
        __metafn_is_private,
        __metafn_is_virtual,
        __metafn_is_pure_virtual,
        __metafn_is_override,
        __metafn_is_deleted,
        __metafn_is_defaulted,
        __metafn_is_explicit,
        __metafn_is_noexcept,
        __metafn_is_bit_field,
        __metafn_is_enumerator,
        __metafn_is_const,
        __metafn_is_volatile,
        __metafn_is_mutable_member,
        __metafn_is_lvalue_reference_qualified,
        __metafn_is_rvalue_reference_qualified,
        __metafn_has_static_storage_duration,
        __metafn_has_thread_storage_duration,
        __metafn_has_automatic_storage_duration,
        __metafn_has_internal_linkage,
        __metafn_has_module_linkage,
        __metafn_has_external_linkage,
        __metafn_has_linkage,
        __metafn_is_class_member,
        __metafn_is_namespace_member,
        __metafn_is_nonstatic_data_member,
        __metafn_is_static_member,
        __metafn_is_base,
        __metafn_is_data_member_spec,
        __metafn_is_namespace,
        __metafn_is_function,
        __metafn_is_variable,
        __metafn_is_type,
        __metafn_is_alias,
        __metafn_is_entity_proxy,
        __metafn_is_complete_type,
        __metafn_has_complete_definition,
        __metafn_is_enumerable_type,
        __metafn_is_template,
        __metafn_is_function_template,
        __metafn_is_variable_template,
        __metafn_is_class_template,
        __metafn_is_alias_template,
        __metafn_is_conversion_function_template,
        __metafn_is_operator_function_template,
        __metafn_is_literal_operator_template,
        __metafn_is_constructor_template,
        __metafn_is_concept,
        __metafn_is_structured_binding,
        __metafn_is_value,
        __metafn_is_object,
        __metafn_has_template_arguments,
        __metafn_has_default_member_initializer,
        __metafn_is_conversion_function,
        __metafn_is_operator_function,
        __metafn_is_literal_operator,
        __metafn_is_constructor,
        __metafn_is_default_constructor,
        __metafn_is_copy_constructor,
        __metafn_is_move_constructor,
        __metafn_is_assignment,
        __metafn_is_copy_assignment,
        __metafn_is_move_assignment,
        __metafn_is_destructor,
        __metafn_is_special_member_function,
        __metafn_is_user_provided,
        __metafn_is_user_declared,
        __metafn_reflect_result,
        __metafn_data_member_spec,
        __metafn_define_aggregate,
        __metafn_offset_of,
        __metafn_size_of,
        __metafn_bit_offset_of,
        __metafn_bit_size_of,
        __metafn_alignment_of,

        // P3096 parameter reflection
        __metafn_get_ith_parameter_of,
        __metafn_has_ellipsis_parameter,
        __metafn_has_default_argument,
        __metafn_is_explicit_object_parameter,
        __metafn_is_function_parameter,
        __metafn_return_type_of,
        __metafn_variable_of,

        // P3394 annotation metafunctions
        __metafn_get_ith_annotation_of,
        __metafn_is_annotation,
        __metafn_annotate,

        // P3493 accessibility
        __metafn_access_context,
        __metafn_is_accessible,

        // Other
        __metafn_is_access_specified,
        __metafn_reflect_invoke,
      };

      //*********************************************************************
      /// Helper: resolve through type aliases to the underlying entity.
      //*********************************************************************
      consteval auto __underlying_entity_of(info r) -> info
      {
        return __metafunction(detail::__metafn_underlying_entity_of, r);
      }

      //*********************************************************************
      /// Helper: force expansion of compiler-builtin types through a
      /// wrapper template so that e.g. __int128 becomes a concrete type.
      //*********************************************************************
      template <class T>
      struct __wrap_workaround
      {
        using type = T;
      };

      consteval auto __workaround_expand_compiler_builtins(info type) -> info;

    } // namespace detail

    //*************************************************************************
    /// __range_of_infos: Iterator infrastructure for member/base/enumerator
    /// queries that return vectors of info. Matches Bloomberg implementation.
    //*************************************************************************
    namespace __range_of_infos
    {
      struct sentinel
      {
      };

      template <typename Front, typename Next, typename Map>
      class iterator
      {
        static constexpr Front m_front = {};
        static constexpr Next  m_next  = {};

        info   m_reflectedEntity{^^sentinel};
        info   m_currInfoItr{^^sentinel};
        Map    m_mapFn;
        size_t m_nextIndex{0};

      public:

        using value_type      = info;
        using reference       = info;
        using pointer         = info;
        using difference_type = ptrdiff_t;

        consteval iterator()
          : m_currInfoItr{^^sentinel}
          , m_mapFn{}
        {
        }

        consteval iterator(info reflectedEntity)
          : m_reflectedEntity{reflectedEntity}
          , m_currInfoItr{m_front(reflectedEntity)}
          , m_nextIndex{1}
        {
        }

        consteval info operator*() const
        {
          return m_mapFn(m_currInfoItr);
        }

        consteval iterator& operator++()
        {
          m_currInfoItr = m_next(m_currInfoItr, m_reflectedEntity, m_nextIndex++);
          return *this;
        }

        consteval iterator operator++(int)
        {
          iterator tmp = *this;
          operator++();
          return tmp;
        }

        consteval friend bool operator==(iterator a, iterator b)
        {
          return a.m_currInfoItr == b.m_currInfoItr;
        }
        consteval friend bool operator!=(iterator a, iterator b)
        {
          return a.m_currInfoItr != b.m_currInfoItr;
        }
      };

      template <typename Iter>
      class range
      {
        Iter m_first;
        Iter m_last;

      public:

        using iterator = Iter;

        consteval range(info reflection)
          : m_first(reflection)
          , m_last()
        {
        }

        consteval iterator begin() const
        {
          return m_first;
        }
        consteval iterator end() const
        {
          return m_last;
        }

        consteval size_t size() const
        {
          size_t n = 0;
          for (auto it = m_first; it != m_last; ++it) ++n;
          return n;
        }

        consteval info operator[](size_t idx) const
        {
          auto it = m_first;
          for (size_t i = 0; i < idx; ++i) ++it;
          return *it;
        }

    #if ETL_USING_STL
        consteval std::vector<info> to_vec() const
        {
          return std::vector<info>(m_first, m_last);
        }
    #endif
      };

      // Functors for member iteration
      struct front_member_of_fn
      {
        consteval info operator()(info reflectedEntity) const
        {
          return __metafunction(detail::__metafn_get_begin_member_decl_of, reflectedEntity, ^^sentinel);
        }
      };
      struct next_member_of_fn
      {
        consteval info operator()(info currItrInfo, auto, auto) const
        {
          return __metafunction(detail::__metafn_get_next_member_decl_of, currItrInfo, ^^sentinel);
        }
      };
      struct map_decl_to_entity_fn
      {
        consteval info operator()(info reflectedDecl) const
        {
          return __metafunction(detail::__metafn_map_decl_to_entity, reflectedDecl);
        }
      };
      struct map_identity_fn
      {
        consteval info operator()(info reflectedDecl) const
        {
          return reflectedDecl;
        }
      };

      // Functors for template argument iteration
      struct front_targ_fn
      {
        consteval info operator()(info reflectedEntity) const
        {
          return __metafunction(detail::__metafn_get_ith_template_argument_of, reflectedEntity, ^^sentinel, 0);
        }
      };
      struct next_targ_fn
      {
        consteval info operator()(auto, info reflectedEntity, size_t idx) const
        {
          return __metafunction(detail::__metafn_get_ith_template_argument_of, reflectedEntity, ^^sentinel, idx);
        }
      };

      // Functors for base class iteration
      struct front_base_of_fn
      {
        consteval info operator()(info reflectedEntity) const
        {
          return __metafunction(detail::__metafn_get_ith_base_of, reflectedEntity, ^^sentinel, 0);
        }
      };
      struct next_base_of_fn
      {
        consteval info operator()(auto, info reflectedEntity, size_t idx) const
        {
          return __metafunction(detail::__metafn_get_ith_base_of, reflectedEntity, ^^sentinel, idx);
        }
      };

      // Functors for enumerator iteration
      struct front_enumerator_of_fn
      {
        consteval info operator()(info reflectedEntity) const
        {
          return __metafunction(detail::__metafn_get_begin_enumerator_decl_of, reflectedEntity, ^^sentinel);
        }
      };
      struct next_enumerator_of_fn
      {
        consteval info operator()(info currItrInfo, auto, auto) const
        {
          return __metafunction(detail::__metafn_get_get_next_enumerator_decl_of, currItrInfo, ^^sentinel);
        }
      };

    #if ETL_HAS_PARAMETER_REFLECTION
      // Functors for parameter iteration
      struct front_parameter_of_fn
      {
        consteval info operator()(info reflectedEntity) const
        {
          return __metafunction(detail::__metafn_get_ith_parameter_of, reflectedEntity, ^^sentinel, 0);
        }
      };
      struct next_parameter_of_fn
      {
        consteval info operator()(auto, info reflectedEntity, size_t idx) const
        {
          return __metafunction(detail::__metafn_get_ith_parameter_of, reflectedEntity, ^^sentinel, idx);
        }
      };
    #endif

    } // namespace __range_of_infos

    //*************************************************************************
    // operators enum
    //*************************************************************************
    enum class operators
    {
      op_new = 1,
      op_delete,
      op_array_new,
      op_array_delete,
      op_co_await,
      op_parentheses,
      op_square_brackets,
      op_arrow,
      op_arrow_star,
      op_tilde,
      op_exclamation,
      op_plus,
      op_minus,
      op_star,
      op_slash,
      op_percent,
      op_caret,
      op_ampersand,
      op_pipe,
      op_equals,
      op_plus_equals,
      op_minus_equals,
      op_star_equals,
      op_slash_equals,
      op_percent_equals,
      op_caret_equals,
      op_ampersand_equals,
      op_pipe_equals,
      op_equals_equals,
      op_exclamation_equals,
      op_less,
      op_greater,
      op_less_equals,
      op_greater_equals,
      op_spaceship,
      op_ampersand_ampersand,
      op_pipe_pipe,
      op_less_less,
      op_greater_greater,
      op_less_less_equals,
      op_greater_greater_equals,
      op_plus_plus,
      op_minus_minus,
      op_comma,
    };
    using enum operators;

    //*************************************************************************
    /// Returns the identifier for the reflected entity.
    //*************************************************************************
    consteval auto identifier_of(info r) -> etl::string_view
    {
      return __metafunction(detail::__metafn_identifier_of, ^^const char*, r,
                            /*UTF8=*/false, /*EnforceConsistent=*/true);
    }

    //*************************************************************************
    /// Returns whether the reflected entity has an associated identifier.
    //*************************************************************************
    consteval auto has_identifier(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_identifier, r);
    }

    //*************************************************************************
    /// Returns an implementation-defined display name for the reflected entity.
    //*************************************************************************
    consteval auto display_string_of(info r) -> etl::string_view;

    //*************************************************************************
    /// Returns the operator for the reflected operator function.
    //*************************************************************************
    consteval auto operator_of(info r) -> operators
    {
      return operators(__metafunction(detail::__metafn_operator_of, r));
    }

    //*************************************************************************
    /// Returns the string representation of the given operator.
    //*************************************************************************
    consteval auto symbol_of(operators op) -> etl::string_view
    {
      static constexpr etl::string_view op_names[45] = {{},   "new", "delete", "new[]", "delete[]", "co_await", "()", "[]", "->", "->*", "~",  "!",
                                                        "+",  "-",   "*",      "/",     "%",        "^",        "&",  "|",  "=",  "+=",  "-=", "*=",
                                                        "/=", "%=",  "^=",     "&=",    "|=",       "==",       "!=", "<",  ">",  "<=",  ">=", "<=>",
                                                        "&&", "||",  "<<",     ">>",    "<<=",      ">>=",      "++", "--", ","};
      return op_names[int(op)];
    }

    //*************************************************************************
    // Core entity queries
    //*************************************************************************

    //*************************************************************************
    /// Returns a reflection of the type of the reflected entity.
    //*************************************************************************
    consteval auto type_of(info r) -> info
    {
      return __metafunction(detail::__metafn_type_of, r);
    }

    //*************************************************************************
    /// Returns the parent (containing class or namespace) of the entity.
    //*************************************************************************
    consteval auto parent_of(info r) -> info
    {
      return __metafunction(detail::__metafn_parent_of, r);
    }

    //*************************************************************************
    /// Returns a reflection of the canonical type for a type alias.
    //*************************************************************************
    consteval auto dealias(info r) -> info
    {
      return __metafunction(detail::__metafn_underlying_entity_of, r);
    }

    //*************************************************************************
    /// Returns a reflection of the object designated by the reflected entity.
    //*************************************************************************
    consteval auto object_of(info r) -> info
    {
      return __metafunction(detail::__metafn_object_of, r);
    }

    //*************************************************************************
    /// Returns a reflection of the value evaluated from the reflected entity.
    //*************************************************************************
    consteval auto constant_of(info r) -> info
    {
      return __metafunction(detail::__metafn_constant_of, r);
    }

    //*************************************************************************
    /// Returns the template from which the reflected entity was instantiated.
    //*************************************************************************
    consteval auto template_of(info r) -> info
    {
      return __metafunction(detail::__metafn_template_of, r);
    }

        //*************************************************************************
        /// Returns the template arguments of the reflected specialization.
        //*************************************************************************
    #if ETL_USING_STL
    consteval auto template_arguments_of(info r) -> std::vector<info>
    {
      using iterator =
        __range_of_infos::iterator< __range_of_infos::front_targ_fn, __range_of_infos::next_targ_fn, __range_of_infos::map_identity_fn>;
      using range = __range_of_infos::range<iterator>;

      auto rng = range{r};
      return std::vector<info>{rng.begin(), rng.end()};
    }
    #endif

    //*************************************************************************
    // Entity classification predicates (Step 6)
    // Each wraps a single __metafunction() call returning bool.
    //*************************************************************************

    // --- Access specifiers ---

    consteval auto is_public(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_public, r);
    }

    consteval auto is_protected(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_protected, r);
    }

    consteval auto is_private(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_private, r);
    }

    // --- Virtual / override ---

    consteval auto is_virtual(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_virtual, r);
    }

    consteval auto is_pure_virtual(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_pure_virtual, r);
    }

    consteval auto is_override(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_override, r);
    }

    // --- Function properties ---

    consteval auto is_deleted(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_deleted, r);
    }

    consteval auto is_defaulted(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_defaulted, r);
    }

    consteval auto is_explicit(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_explicit, r);
    }

    consteval auto is_noexcept(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_noexcept, r);
    }

    // --- Data member properties ---

    consteval auto is_bit_field(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_bit_field, r);
    }

    consteval auto is_enumerator(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_enumerator, r);
    }

    // --- CV / ref qualification ---

    consteval auto is_const(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_const, r);
    }

    consteval auto is_volatile(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_volatile, r);
    }

    consteval auto is_mutable_member(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_mutable_member, r);
    }

    consteval auto is_lvalue_reference_qualified(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_lvalue_reference_qualified, r);
    }

    consteval auto is_rvalue_reference_qualified(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_rvalue_reference_qualified, r);
    }

    // --- Storage duration ---

    consteval auto has_static_storage_duration(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_static_storage_duration, r);
    }

    consteval auto has_thread_storage_duration(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_thread_storage_duration, r);
    }

    consteval auto has_automatic_storage_duration(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_automatic_storage_duration, r);
    }

    // --- Linkage ---

    consteval auto has_internal_linkage(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_internal_linkage, r);
    }

    consteval auto has_module_linkage(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_module_linkage, r);
    }

    consteval auto has_external_linkage(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_external_linkage, r);
    }

    consteval auto has_linkage(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_linkage, r);
    }

    // --- Membership ---

    consteval auto is_class_member(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_class_member, r);
    }

    consteval auto is_namespace_member(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_namespace_member, r);
    }

    consteval auto is_nonstatic_data_member(info r) -> bool
    {
      return is_class_member(r) && __metafunction(detail::__metafn_is_nonstatic_data_member, r);
    }

    consteval auto is_static_member(info r) -> bool
    {
      return is_class_member(r) && __metafunction(detail::__metafn_is_static_member, r);
    }

    consteval auto is_base(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_base, r);
    }

    consteval auto is_data_member_spec(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_data_member_spec, r);
    }

    // --- Entity kind ---

    consteval auto is_namespace(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_namespace, r);
    }

    consteval auto is_function(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_function, r);
    }

    consteval auto is_variable(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_variable, r);
    }

    consteval auto is_type(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_type, r);
    }

    consteval auto is_type_alias(info r) -> bool
    {
      return is_type(r) && __metafunction(detail::__metafn_is_alias, r);
    }

    consteval auto is_namespace_alias(info r) -> bool
    {
      return is_namespace(r) && __metafunction(detail::__metafn_is_alias, r);
    }

    consteval auto is_complete_type(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_complete_type, r);
    }

    consteval auto is_enumerable_type(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_enumerable_type, r);
    }

    // --- Templates ---

    consteval auto is_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_template, r);
    }

    consteval auto is_function_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_function_template, r);
    }

    consteval auto is_variable_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_variable_template, r);
    }

    consteval auto is_class_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_class_template, r);
    }

    consteval auto is_alias_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_alias_template, r);
    }

    consteval auto is_conversion_function_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_conversion_function_template, r);
    }

    consteval auto is_operator_function_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_operator_function_template, r);
    }

    consteval auto is_literal_operator_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_literal_operator_template, r);
    }

    consteval auto is_constructor_template(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_constructor_template, r);
    }

    consteval auto is_concept(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_concept, r);
    }

    // --- Value / object ---

    consteval auto is_structured_binding(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_structured_binding, r);
    }

    consteval auto is_value(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_value, r);
    }

    consteval auto is_object(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_object, r);
    }

    consteval auto has_template_arguments(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_template_arguments, r);
    }

    consteval auto has_default_member_initializer(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_default_member_initializer, r);
    }

    // --- Special member functions ---

    consteval auto is_conversion_function(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_conversion_function, r);
    }

    consteval auto is_operator_function(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_operator_function, r);
    }

    consteval auto is_literal_operator(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_literal_operator, r);
    }

    consteval auto is_constructor(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_constructor, r);
    }

    consteval auto is_default_constructor(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_default_constructor, r);
    }

    consteval auto is_copy_constructor(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_copy_constructor, r);
    }

    consteval auto is_move_constructor(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_move_constructor, r);
    }

    consteval auto is_assignment(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_assignment, r);
    }

    consteval auto is_copy_assignment(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_copy_assignment, r);
    }

    consteval auto is_move_assignment(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_move_assignment, r);
    }

    consteval auto is_destructor(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_destructor, r);
    }

    consteval auto is_special_member_function(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_special_member_function, r);
    }

    consteval auto is_user_provided(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_user_provided, r);
    }

    consteval auto is_user_declared(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_user_declared, r);
    }

    consteval auto is_structural_type(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_structural_type, r);
    }

    //*************************************************************************
    // Access context & member queries (Step 7)
    //*************************************************************************

    //*************************************************************************
    /// Represents a context from which accessibility is checked.
    //*************************************************************************
    class access_context
    {
      consteval access_context(info scope, info designating_class) noexcept
        : d_scope{scope}
        , d_designating_class{designating_class}
      {
      }

    public:

      const info d_scope;
      const info d_designating_class;

      consteval access_context()                               = delete;
      consteval access_context(const access_context&) noexcept = default;
      consteval access_context(access_context&&) noexcept      = default;

      consteval info scope() const
      {
        return d_scope;
      }
      consteval info designating_class() const
      {
        return d_designating_class;
      }

      [[clang::instantiation_dependent]]
      static consteval access_context current() noexcept
      {
        return {__metafunction(detail::__metafn_access_context), {}};
      }

      static consteval access_context unprivileged() noexcept
      {
        return access_context{^^::, info{}};
      }

      static consteval access_context unchecked() noexcept
      {
        return access_context{info{}, info{}};
      }

      consteval access_context via(info new_designating_class) const
      {
        return access_context{d_scope, new_designating_class};
      }
    };

    //*************************************************************************
    /// Returns whether the reflected entity is accessible from the context.
    //*************************************************************************
    consteval auto is_accessible(info r, access_context ctx) -> bool
    {
      if (!is_class_member(r) && !is_base(r) && !is_enumerator(r))
        return true;

      return __metafunction(detail::__metafn_is_accessible, r, ctx.scope(), ctx.designating_class());
    }

    //*************************************************************************
    /// A consteval fixed-capacity array of info values (no STL required).
    /// Used as the return type of collection-returning functions in the
    /// non-STL path.
    //*************************************************************************
    struct info_array
    {
      static constexpr size_t max_capacity = 256;

      info   data_[max_capacity]{};
      size_t size_ = 0;

      consteval info_array() = default;

      consteval size_t size() const
      {
        return size_;
      }
      consteval bool empty() const
      {
        return size_ == 0;
      }

      consteval info operator[](size_t i) const
      {
        return data_[i];
      }
      consteval info& operator[](size_t i)
      {
        return data_[i];
      }

      consteval const info* begin() const
      {
        return data_;
      }
      consteval const info* end() const
      {
        return data_ + size_;
      }
      consteval info* begin()
      {
        return data_;
      }
      consteval info* end()
      {
        return data_ + size_;
      }

      consteval const info* data() const
      {
        return data_;
      }
      consteval info* data()
      {
        return data_;
      }

      consteval void push_back(info i)
      {
        data_[size_++] = i;
      }

      consteval void insert_front(info i)
      {
        for (size_t j = size_; j > 0; --j) data_[j] = data_[j - 1];
        data_[0] = i;
        ++size_;
      }
    };

    namespace detail
    {
      template <class P>
      consteval auto __filtered_array(info_array v, P pred) -> info_array
      {
        info_array result;
        for (size_t i = 0; i < v.size(); ++i)
        {
          if (pred(v[i]))
            result.push_back(v[i]);
        }
        return result;
      }

      template <class Range>
      consteval auto __range_to_array(Range rng) -> info_array
      {
        info_array result;
        for (auto it = rng.begin(); it != rng.end(); ++it) result.push_back(*it);
        return result;
      }
    } // namespace detail

        //*************************************************************************
        /// Helper and collection-returning functions — require STL (std::vector).
        //*************************************************************************
    #if ETL_USING_STL
    namespace detail
    {
      template <class P>
      consteval auto __filtered(std::vector<info> v, P pred) -> std::vector<info>
      {
        std::erase_if(v, [=](info i) { return !pred(i); });
        return v;
      }
    } // namespace detail

    //*************************************************************************
    /// Returns accessible members of the reflected class/namespace.
    //*************************************************************************
    consteval auto members_of(info r, access_context ctx) -> std::vector<info>
    {
      using iterator = __range_of_infos::iterator< __range_of_infos::front_member_of_fn, __range_of_infos::next_member_of_fn,
                                                   __range_of_infos::map_decl_to_entity_fn>;
      using range    = __range_of_infos::range<iterator>;

      return detail::__filtered(range(r).to_vec(), [=](info i) { return is_accessible(i, ctx); });
    }

    //*************************************************************************
    /// Returns accessible base classes of the reflected class.
    //*************************************************************************
    consteval auto bases_of(info r, access_context ctx) -> std::vector<info>
    {
      if (is_namespace(r))
        throw "Namespaces cannot have base classes";

      using iterator =
        __range_of_infos::iterator< __range_of_infos::front_base_of_fn, __range_of_infos::next_base_of_fn, __range_of_infos::map_identity_fn>;
      using range = __range_of_infos::range<iterator>;

      return detail::__filtered(range(r).to_vec(), [=](info i) { return is_accessible(i, ctx); });
    }

    //*************************************************************************
    /// Returns accessible static data members of the reflected class.
    //*************************************************************************
    consteval auto static_data_members_of(info r, access_context ctx) -> std::vector<info>
    {
      if (is_namespace(r))
        throw "Namespaces cannot have static data members";

      return detail::__filtered(members_of(r, ctx), is_variable);
    }

    //*************************************************************************
    /// Returns accessible non-static data members of the reflected class.
    //*************************************************************************
    consteval auto nonstatic_data_members_of(info r, access_context ctx) -> std::vector<info>
    {
      if (is_namespace(r))
        throw "Namespaces cannot have non-static data members";

      return detail::__filtered(members_of(r, ctx), is_nonstatic_data_member);
    }

    //*************************************************************************
    /// Returns the enumerators of the reflected enumeration type.
    //*************************************************************************
    consteval auto enumerators_of(info r) -> std::vector<info>
    {
      if (!is_enumerable_type(r))
        throw "Reflection must represent an enumeration with a definition";

      using iterator = __range_of_infos::iterator< __range_of_infos::front_enumerator_of_fn, __range_of_infos::next_enumerator_of_fn,
                                                   __range_of_infos::map_identity_fn>;
      using range    = __range_of_infos::range<iterator>;

      auto rng = range{r};
      return std::vector<info>{rng.begin(), rng.end()};
    }

    //*************************************************************************
    /// Convenience: check for inaccessible members/bases.
    //*************************************************************************
    consteval auto has_inaccessible_nonstatic_data_members(info r, access_context ctx) -> bool
    {
      auto all = nonstatic_data_members_of(r, access_context::unchecked());
      for (info i : all)
      {
        if (!is_accessible(i, ctx))
          return true;
      }
      return false;
    }

    consteval auto has_inaccessible_bases(info r, access_context ctx) -> bool
    {
      auto all = bases_of(r, access_context::unchecked());
      for (info i : all)
      {
        if (!is_accessible(i, ctx))
          return true;
      }
      return false;
    }
    #endif // ETL_USING_STL (collection-returning functions)

    #if !ETL_USING_STL
    //*************************************************************************
    /// Non-STL collection-returning functions — return info_array.
    //*************************************************************************

    //*************************************************************************
    /// Returns all accessible members (via info_array).
    //*************************************************************************
    consteval auto members_of(info r, access_context ctx) -> info_array
    {
      using iterator = __range_of_infos::iterator< __range_of_infos::front_member_of_fn, __range_of_infos::next_member_of_fn,
                                                   __range_of_infos::map_decl_to_entity_fn>;
      using range    = __range_of_infos::range<iterator>;

      return detail::__filtered_array(detail::__range_to_array(range(r)), [=](info i) { return is_accessible(i, ctx); });
    }

    //*************************************************************************
    /// Returns accessible base classes (via info_array).
    //*************************************************************************
    consteval auto bases_of(info r, access_context ctx) -> info_array
    {
      if (is_namespace(r))
        throw "Namespaces cannot have base classes";

      using iterator =
        __range_of_infos::iterator< __range_of_infos::front_base_of_fn, __range_of_infos::next_base_of_fn, __range_of_infos::map_identity_fn>;
      using range = __range_of_infos::range<iterator>;

      return detail::__filtered_array(detail::__range_to_array(range(r)), [=](info i) { return is_accessible(i, ctx); });
    }

    //*************************************************************************
    /// Returns accessible static data members (via info_array).
    //*************************************************************************
    consteval auto static_data_members_of(info r, access_context ctx) -> info_array
    {
      if (is_namespace(r))
        throw "Namespaces cannot have static data members";

      return detail::__filtered_array(members_of(r, ctx), is_variable);
    }

    //*************************************************************************
    /// Returns accessible non-static data members (via info_array).
    //*************************************************************************
    consteval auto nonstatic_data_members_of(info r, access_context ctx) -> info_array
    {
      if (is_namespace(r))
        throw "Namespaces cannot have non-static data members";

      return detail::__filtered_array(members_of(r, ctx), is_nonstatic_data_member);
    }

    //*************************************************************************
    /// Returns enumerators of the enumeration type (via info_array).
    //*************************************************************************
    consteval auto enumerators_of(info r) -> info_array
    {
      if (!is_enumerable_type(r))
        throw "Reflection must represent an enumeration with a definition";

      using iterator = __range_of_infos::iterator< __range_of_infos::front_enumerator_of_fn, __range_of_infos::next_enumerator_of_fn,
                                                   __range_of_infos::map_identity_fn>;
      using range    = __range_of_infos::range<iterator>;

      return detail::__range_to_array(range{r});
    }

    //*************************************************************************
    /// Returns template arguments of the reflected specialization (via info_array).
    //*************************************************************************
    consteval auto template_arguments_of(info r) -> info_array
    {
      using iterator =
        __range_of_infos::iterator< __range_of_infos::front_targ_fn, __range_of_infos::next_targ_fn, __range_of_infos::map_identity_fn>;
      using range = __range_of_infos::range<iterator>;

      return detail::__range_to_array(range{r});
    }

    //*************************************************************************
    /// Convenience: check for inaccessible members/bases.
    //*************************************************************************
    consteval auto has_inaccessible_nonstatic_data_members(info r, access_context ctx) -> bool
    {
      auto all = nonstatic_data_members_of(r, access_context::unchecked());
      for (size_t i = 0; i < all.size(); ++i)
      {
        if (!is_accessible(all[i], ctx))
          return true;
      }
      return false;
    }

    consteval auto has_inaccessible_bases(info r, access_context ctx) -> bool
    {
      auto all = bases_of(r, access_context::unchecked());
      for (size_t i = 0; i < all.size(); ++i)
      {
        if (!is_accessible(all[i], ctx))
          return true;
      }
      return false;
    }
    #endif // !ETL_USING_STL (collection-returning functions)

    #if ETL_HAS_PARAMETER_REFLECTION
      #if ETL_USING_STL
    //*************************************************************************
    /// Returns parameters of the reflected function.
    //*************************************************************************
    consteval auto parameters_of(info r) -> std::vector<info>
    {
      using iterator = __range_of_infos::iterator< __range_of_infos::front_parameter_of_fn, __range_of_infos::next_parameter_of_fn,
                                                   __range_of_infos::map_identity_fn>;
      using range    = __range_of_infos::range<iterator>;

      auto rng = range{r};
      return std::vector<info>{rng.begin(), rng.end()};
    }
      #else  // !ETL_USING_STL
    //*************************************************************************
    /// Returns parameters of the reflected function (non-STL, via info_array).
    //*************************************************************************
    consteval auto parameters_of(info r) -> info_array
    {
      using iterator = __range_of_infos::iterator< __range_of_infos::front_parameter_of_fn, __range_of_infos::next_parameter_of_fn,
                                                   __range_of_infos::map_identity_fn>;
      using range    = __range_of_infos::range<iterator>;

      return detail::__range_to_array(range{r});
    }
      #endif // ETL_USING_STL

    consteval auto has_ellipsis_parameter(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_ellipsis_parameter, r);
    }

    consteval auto has_default_argument(info r) -> bool
    {
      return __metafunction(detail::__metafn_has_default_argument, r);
    }

    consteval auto is_explicit_object_parameter(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_explicit_object_parameter, r);
    }

    consteval auto is_function_parameter(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_function_parameter, r);
    }

    consteval auto return_type_of(info r) -> info
    {
      return __metafunction(detail::__metafn_return_type_of, r);
    }
    #endif   // ETL_HAS_PARAMETER_REFLECTION

        //*************************************************************************
        // substitute, can_substitute (Step 8a)
        // These require STL types (std::vector, std::initializer_list).
        //*************************************************************************

    #if ETL_USING_STL

    //*************************************************************************
    /// Returns whether 'templ' substituted with 'args' forms a valid
    /// template-id.
    //*************************************************************************
    template <reflection_range R = std::initializer_list<info> >
    consteval auto can_substitute(info templ, R&& args) -> bool
    {
      info              sub;
      std::vector<info> vargs(args.begin(), args.end());
      sub = __metafunction(detail::__metafn_substitute, templ, vargs.data(), vargs.size(), false);
      return sub != info{};
    }

    //*************************************************************************
    /// Returns a reflection representing the template instantiation of
    /// 'templ' with the reflected entities in 'args'.
    //*************************************************************************
    template <reflection_range R = std::initializer_list<info> >
    consteval auto substitute(info templ, R&& args) -> info
    {
      std::vector<info> vargs(args.begin(), args.end());
      return __metafunction(detail::__metafn_substitute, templ, vargs.data(), vargs.size(), true);
    }

    #endif // ETL_USING_STL (substitute, can_substitute)

    #if !ETL_USING_STL

    //*************************************************************************
    /// Non-STL substitute: accepts an info_array of arguments.
    //*************************************************************************
    consteval auto substitute(info templ, info_array args) -> info
    {
      return __metafunction(detail::__metafn_substitute, templ, args.data(), args.size(), true);
    }

    //*************************************************************************
    /// Non-STL substitute: accepts a brace-init list via C array.
    //*************************************************************************
    template <size_t N>
    consteval auto substitute(info templ, const info (&args)[N]) -> info
    {
      return __metafunction(detail::__metafn_substitute, templ, &args[0], N, true);
    }

    //*************************************************************************
    /// Non-STL can_substitute: accepts an info_array of arguments.
    //*************************************************************************
    consteval auto can_substitute(info templ, info_array args) -> bool
    {
      info sub = __metafunction(detail::__metafn_substitute, templ, args.data(), args.size(), false);
      return sub != info{};
    }

    //*************************************************************************
    /// Non-STL can_substitute: accepts a brace-init list via C array.
    //*************************************************************************
    template <size_t N>
    consteval auto can_substitute(info templ, const info (&args)[N]) -> bool
    {
      info sub = __metafunction(detail::__metafn_substitute, templ, &args[0], N, false);
      return sub != info{};
    }

    #endif // !ETL_USING_STL (substitute, can_substitute)

    //*************************************************************************
    /// Extracts a value of type T from a reflection of a value or object.
    //*************************************************************************
    template <typename Ty>
      requires(!etl::is_rvalue_reference_v<Ty>)
    consteval auto extract(info r) -> Ty
    {
      return __metafunction(detail::__metafn_extract, ^^Ty, r);
    }

        //*************************************************************************
        // Type trait wrappers (Step 9) — require substitute (STL-only).
        //*************************************************************************

    #if ETL_USING_STL

    // --- Primary type categories ---

    consteval auto is_void_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_void_v, {r}));
    }

    consteval auto is_null_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_null_pointer_v, {r}));
    }

    consteval auto is_integral_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_integral_v, {r}));
    }

    consteval auto is_floating_point_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_floating_point_v, {r}));
    }

    consteval auto is_array_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_array_v, {r}));
    }

    consteval auto is_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_pointer_v, {r}));
    }

    consteval auto is_lvalue_reference_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_lvalue_reference_v, {r}));
    }

    consteval auto is_rvalue_reference_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_rvalue_reference_v, {r}));
    }

    consteval auto is_member_object_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_member_object_pointer_v, {r}));
    }

    consteval auto is_member_function_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_member_function_pointer_v, {r}));
    }

    consteval auto is_enum_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_enum_v, {r}));
    }

    consteval auto is_union_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_union_v, {r}));
    }

    consteval auto is_class_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_class_v, {r}));
    }

    consteval auto is_function_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_function_v, {r}));
    }

    consteval auto is_reflection_type(info r) -> bool
    {
      return r == ^^info;
    }

    // --- Composite type categories ---

    consteval auto is_reference_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_reference_v, {r}));
    }

    consteval auto is_arithmetic_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_arithmetic_v, {r}));
    }

    consteval auto is_fundamental_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_fundamental_v, {r}));
    }

    consteval auto is_object_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_object_v, {r}));
    }

    consteval auto is_scalar_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_scalar_v, {r}));
    }

    consteval auto is_compound_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_compound_v, {r}));
    }

    consteval auto is_member_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_member_pointer_v, {r}));
    }

    // --- Type properties ---

    consteval auto is_const_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_const_v, {r}));
    }

    consteval auto is_volatile_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_volatile_v, {r}));
    }

    consteval auto is_trivially_copyable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_trivially_copyable_v, {r}));
    }

    consteval auto is_empty_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_empty_v, {r}));
    }

    consteval auto is_polymorphic_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_polymorphic_v, {r}));
    }

    consteval auto is_abstract_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_abstract_v, {r}));
    }

    consteval auto is_final_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_final_v, {r}));
    }

    consteval auto is_aggregate_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_aggregate_v, {r}));
    }

    consteval auto is_signed_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_signed_v, {r}));
    }

    consteval auto is_unsigned_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_unsigned_v, {r}));
    }

    consteval auto is_bounded_array_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_bounded_array_v, {r}));
    }

    consteval auto is_unbounded_array_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_unbounded_array_v, {r}));
    }

    consteval auto is_scoped_enum_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_scoped_enum_v, {r}));
    }

    // --- Constructible / assignable / destructible wrappers ---

    template <reflection_range R = std::initializer_list<info> >
    consteval auto is_constructible_type(info type, R&& type_args) -> bool
    {
      std::vector<info> args(type_args.begin(), type_args.end());
      args.insert(args.begin(), type);
      return extract<bool>(substitute(^^etl::is_constructible_v, args));
    }

    consteval auto is_default_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_default_constructible_v, {r}));
    }

    consteval auto is_copy_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_copy_constructible_v, {r}));
    }

    consteval auto is_move_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_move_constructible_v, {r}));
    }

    consteval auto is_assignable_type(info dst, info src) -> bool
    {
      return extract<bool>(substitute(^^etl::is_assignable_v, {dst, src}));
    }

    consteval auto is_copy_assignable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_copy_assignable_v, {r}));
    }

    consteval auto is_move_assignable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_move_assignable_v, {r}));
    }

    consteval auto is_destructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_destructible_v, {r}));
    }

    // --- Trivially constructible/assignable/destructible ---

    template <reflection_range R = std::initializer_list<info> >
    consteval auto is_trivially_constructible_type(info type, R&& type_args) -> bool
    {
      std::vector<info> args(type_args.begin(), type_args.end());
      args.insert(args.begin(), type);
      return extract<bool>(substitute(^^etl::is_trivially_constructible_v, args));
    }

    consteval auto is_trivially_copy_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_trivially_copy_constructible_v, {r}));
    }

    consteval auto is_trivially_copy_assignable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_trivially_copy_assignable_v, {r}));
    }

    consteval auto is_trivially_destructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_trivially_destructible_v, {r}));
    }

    // --- Type property queries ---

    consteval auto rank(info r) -> size_t
    {
      return extract<size_t>(substitute(^^etl::rank_v, {r}));
    }

    consteval auto extent(info r, unsigned i = 0) -> size_t
    {
      return extract<size_t>(substitute(^^etl::extent_v, {r, reflect_constant(i)}));
    }

    // --- Type relations ---

    consteval auto is_same_type(info r, info s) -> bool
    {
      return extract<bool>(substitute(^^etl::is_same_v, {r, s}));
    }

    consteval auto is_base_of_type(info r, info s) -> bool
    {
      return extract<bool>(substitute(^^etl::is_base_of_v, {r, s}));
    }

    consteval auto is_convertible_type(info r, info s) -> bool
    {
      return extract<bool>(substitute(^^etl::is_convertible_v, {r, s}));
    }

    consteval auto has_virtual_destructor(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::has_virtual_destructor_v, {r}));
    }

    //*************************************************************************
    // Implementation of __workaround_expand_compiler_builtins.
    // Wraps a type in __wrap_workaround<T> then extracts the ::type member
    // to force the compiler to produce a canonical type representation.
    //*************************************************************************
    namespace detail
    {
      // Forward declarations needed — members_of and access_context are
      // already declared above, so this definition can see them.
      inline consteval auto __workaround_expand_compiler_builtins(info type) -> info
      {
        auto r = substitute(^^detail::__wrap_workaround, {type});
        r      = members_of(r, access_context::unchecked())[0];
        return detail::__underlying_entity_of(r);
      }
    } // namespace detail

    //*************************************************************************
    // Type transformations (Step 10)
    // Each uses substitute(^^etl::trait_t, {type}) then resolves aliases.
    //*************************************************************************

    consteval auto remove_const(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_const_t, {type})));
    }

    consteval auto remove_volatile(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_volatile_t, {type})));
    }

    consteval auto remove_cv(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_cv_t, {type})));
    }

    consteval auto add_const(info type) -> info
    {
      return detail::__underlying_entity_of(substitute(^^etl::add_const_t, {type}));
    }

    consteval auto add_volatile(info type) -> info
    {
      return detail::__underlying_entity_of(substitute(^^etl::add_volatile_t, {type}));
    }

    consteval auto add_cv(info type) -> info
    {
      return detail::__underlying_entity_of(substitute(^^etl::add_cv_t, {type}));
    }

    consteval auto remove_reference(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_reference_t, {type})));
    }

    consteval auto add_lvalue_reference(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::add_lvalue_reference_t, {type})));
    }

    consteval auto add_rvalue_reference(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::add_rvalue_reference_t, {type})));
    }

    consteval auto make_signed(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::make_signed_t, {type})));
    }

    consteval auto make_unsigned(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::make_unsigned_t, {type})));
    }

    consteval auto remove_extent(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_extent_t, {type})));
    }

    consteval auto remove_all_extents(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_all_extents_t, {type})));
    }

    consteval auto remove_pointer(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_pointer_t, {type})));
    }

    consteval auto add_pointer(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::add_pointer_t, {type})));
    }

    consteval auto remove_cvref(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_cvref_t, {type})));
    }

    consteval auto decay(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::decay_t, {type})));
    }

    template <reflection_range R = std::initializer_list<info> >
    consteval auto common_type(R&& args) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::common_type_t, args)));
    }

    consteval auto underlying_type(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::underlying_type_t, {type})));
    }

    #endif // ETL_USING_STL (type trait wrappers + transformations)

    #if !ETL_USING_STL

    //*************************************************************************
    // Non-STL type trait wrappers + transformations.
    // Same logic as the STL path but uses the C-array substitute overload.
    //*************************************************************************

    namespace detail
    {
      // Variadic helper for is_trivially_constructible_v using compiler builtin.
      // ETL's non-STL is_trivially_constructible_v is single-arg, but the standard
      // (and reflection substitute) needs a variadic version.
      template <typename T, typename... TArgs>
      inline constexpr bool is_trivially_constructible_v_ = __is_trivially_constructible(T, TArgs...);

      // Same for is_nothrow_constructible — ETL non-STL lacks variadic version.
      template <typename T, typename... TArgs>
      inline constexpr bool is_nothrow_constructible_v_ = __is_nothrow_constructible(T, TArgs...);

      // Same for is_constructible — ETL non-STL may not have variadic _v.
      template <typename T, typename... TArgs>
      inline constexpr bool is_constructible_v_ = __is_constructible(T, TArgs...);

      // ETL non-STL fallback uses heuristics for these traits; use compiler builtins directly.
      template <typename T>
      inline constexpr bool is_trivially_copy_constructible_v_ = __is_trivially_constructible(T, const T&);

      template <typename T>
      inline constexpr bool is_trivially_copy_assignable_v_ = __is_trivially_assignable(T&, const T&);

      template <typename T>
      inline constexpr bool is_trivially_destructible_v_ = __is_trivially_destructible(T);

      template <typename T>
      inline constexpr bool is_destructible_v_ = __is_destructible(T);

      // ETL's non-STL decay doesn't handle function-to-function-pointer.
      // Provide a complete decay implementation using add_pointer for functions.
      template <typename T>
      struct decay_helper
      {
        using U = etl::remove_reference_t<T>;

        // Primary: remove CV
        using type = etl::remove_cv_t<U>;
      };

      template <typename T>
        requires etl::is_array<etl::remove_reference_t<T> >::value
      struct decay_helper<T>
      {
        using type = etl::remove_extent_t<etl::remove_reference_t<T> >*;
      };

      template <typename T>
        requires(__is_function(etl::remove_reference_t<T>) && !etl::is_array<etl::remove_reference_t<T> >::value)
      struct decay_helper<T>
      {
        using type = etl::add_pointer_t<etl::remove_reference_t<T> >;
      };

      template <typename T>
      using decay_t_ = typename decay_helper<T>::type;
    } // namespace detail

    // --- Primary type categories ---

    consteval auto is_void_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_void_v, {r}));
    }

    consteval auto is_null_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_null_pointer_v, {r}));
    }

    consteval auto is_integral_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_integral_v, {r}));
    }

    consteval auto is_floating_point_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_floating_point_v, {r}));
    }

    consteval auto is_array_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_array_v, {r}));
    }

    consteval auto is_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_pointer_v, {r}));
    }

    consteval auto is_lvalue_reference_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_lvalue_reference_v, {r}));
    }

    consteval auto is_rvalue_reference_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_rvalue_reference_v, {r}));
    }

    consteval auto is_member_object_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_member_object_pointer_v, {r}));
    }

    consteval auto is_member_function_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_member_function_pointer_v, {r}));
    }

    consteval auto is_enum_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_enum_v, {r}));
    }

    consteval auto is_union_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_union_v, {r}));
    }

    consteval auto is_class_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_class_v, {r}));
    }

    consteval auto is_function_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_function_v, {r}));
    }

    consteval auto is_reflection_type(info r) -> bool
    {
      return r == ^^info;
    }

    // --- Composite type categories ---

    consteval auto is_reference_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_reference_v, {r}));
    }

    consteval auto is_arithmetic_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_arithmetic_v, {r}));
    }

    consteval auto is_fundamental_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_fundamental_v, {r}));
    }

    consteval auto is_object_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_object_v, {r}));
    }

    consteval auto is_scalar_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_scalar_v, {r}));
    }

    consteval auto is_compound_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_compound_v, {r}));
    }

    consteval auto is_member_pointer_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_member_pointer_v, {r}));
    }

    // --- Type properties ---

    consteval auto is_const_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_const_v, {r}));
    }

    consteval auto is_volatile_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_volatile_v, {r}));
    }

    consteval auto is_trivially_copyable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_trivially_copyable_v, {r}));
    }

    consteval auto is_empty_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_empty_v, {r}));
    }

    consteval auto is_polymorphic_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_polymorphic_v, {r}));
    }

    consteval auto is_abstract_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_abstract_v, {r}));
    }

    consteval auto is_final_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_final_v, {r}));
    }

    consteval auto is_aggregate_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_aggregate_v, {r}));
    }

    consteval auto is_signed_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_signed_v, {r}));
    }

    consteval auto is_unsigned_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_unsigned_v, {r}));
    }

    consteval auto is_bounded_array_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_bounded_array_v, {r}));
    }

    consteval auto is_unbounded_array_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_unbounded_array_v, {r}));
    }

    consteval auto is_scoped_enum_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_scoped_enum_v, {r}));
    }

    // --- Constructible / assignable / destructible wrappers ---

    template <size_t N>
    consteval auto is_constructible_type(info type, const info (&type_args)[N]) -> bool
    {
      info_array args;
      args.push_back(type);
      for (size_t i = 0; i < N; ++i) args.push_back(type_args[i]);
      return extract<bool>(substitute(^^etl::is_constructible_v, args));
    }

    consteval auto is_default_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_default_constructible_v, {r}));
    }

    consteval auto is_copy_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_copy_constructible_v, {r}));
    }

    consteval auto is_move_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_move_constructible_v, {r}));
    }

    consteval auto is_assignable_type(info dst, info src) -> bool
    {
      return extract<bool>(substitute(^^etl::is_assignable_v, {dst, src}));
    }

    consteval auto is_copy_assignable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_copy_assignable_v, {r}));
    }

    consteval auto is_move_assignable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::is_move_assignable_v, {r}));
    }

    consteval auto is_destructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^detail::is_destructible_v_, {r}));
    }

    // --- Trivially constructible/assignable/destructible ---

    template <size_t N>
    consteval auto is_trivially_constructible_type(info type, const info (&type_args)[N]) -> bool
    {
      info_array args;
      args.push_back(type);
      for (size_t i = 0; i < N; ++i) args.push_back(type_args[i]);
      return extract<bool>(substitute(^^detail::is_trivially_constructible_v_, args));
    }

    consteval auto is_trivially_copy_constructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^detail::is_trivially_copy_constructible_v_, {r}));
    }

    consteval auto is_trivially_copy_assignable_type(info r) -> bool
    {
      return extract<bool>(substitute(^^detail::is_trivially_copy_assignable_v_, {r}));
    }

    consteval auto is_trivially_destructible_type(info r) -> bool
    {
      return extract<bool>(substitute(^^detail::is_trivially_destructible_v_, {r}));
    }

    // --- Type property queries ---

    consteval auto rank(info r) -> size_t
    {
      return extract<size_t>(substitute(^^etl::rank_v, {r}));
    }

    consteval auto extent(info r, unsigned i = 0) -> size_t
    {
      return extract<size_t>(substitute(^^etl::extent_v, {r, __metafunction(detail::__metafn_reflect_result, ^^unsigned, i)}));
    }

    // --- Type relations ---

    consteval auto is_same_type(info r, info s) -> bool
    {
      return extract<bool>(substitute(^^etl::is_same_v, {r, s}));
    }

    consteval auto is_base_of_type(info r, info s) -> bool
    {
      return extract<bool>(substitute(^^etl::is_base_of_v, {r, s}));
    }

    consteval auto is_convertible_type(info r, info s) -> bool
    {
      return extract<bool>(substitute(^^etl::is_convertible_v, {r, s}));
    }

    consteval auto has_virtual_destructor(info r) -> bool
    {
      return extract<bool>(substitute(^^etl::has_virtual_destructor_v, {r}));
    }

    // --- __workaround_expand_compiler_builtins (non-STL) ---

    namespace detail
    {
      inline consteval auto __workaround_expand_compiler_builtins(info type) -> info
      {
        auto r = substitute(^^detail::__wrap_workaround, {type});
        r      = members_of(r, access_context::unchecked())[0];
        return detail::__underlying_entity_of(r);
      }
    } // namespace detail

    // --- Type transformations ---

    consteval auto remove_const(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_const_t, {type})));
    }

    consteval auto remove_volatile(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_volatile_t, {type})));
    }

    consteval auto remove_cv(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_cv_t, {type})));
    }

    consteval auto add_const(info type) -> info
    {
      return detail::__underlying_entity_of(substitute(^^etl::add_const_t, {type}));
    }

    consteval auto add_volatile(info type) -> info
    {
      return detail::__underlying_entity_of(substitute(^^etl::add_volatile_t, {type}));
    }

    consteval auto add_cv(info type) -> info
    {
      return detail::__underlying_entity_of(substitute(^^etl::add_cv_t, {type}));
    }

    consteval auto remove_reference(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_reference_t, {type})));
    }

    consteval auto add_lvalue_reference(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::add_lvalue_reference_t, {type})));
    }

    consteval auto add_rvalue_reference(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::add_rvalue_reference_t, {type})));
    }

    consteval auto make_signed(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::make_signed_t, {type})));
    }

    consteval auto make_unsigned(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::make_unsigned_t, {type})));
    }

    consteval auto remove_extent(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_extent_t, {type})));
    }

    consteval auto remove_all_extents(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_all_extents_t, {type})));
    }

    consteval auto remove_pointer(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_pointer_t, {type})));
    }

    consteval auto add_pointer(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::add_pointer_t, {type})));
    }

    consteval auto remove_cvref(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::remove_cvref_t, {type})));
    }

    consteval auto decay(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^detail::decay_t_, {type})));
    }

    template <size_t N>
    consteval auto common_type(const info (&args)[N]) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::common_type_t, args)));
    }

    consteval auto common_type(const info_array& args) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::common_type_t, args)));
    }

    consteval auto underlying_type(info type) -> info
    {
      return detail::__workaround_expand_compiler_builtins(detail::__underlying_entity_of(substitute(^^etl::underlying_type_t, {type})));
    }

    #endif // !ETL_USING_STL

    //*************************************************************************
    // Data layout, reflect_constant/object/function (Step 11)
    // These use only __metafunction and do NOT require STL.
    //*************************************************************************

    //*************************************************************************
    /// Represents the offset of a data member within its class.
    //*************************************************************************
    struct member_offset
    {
      ptrdiff_t bytes;
      ptrdiff_t bits;

      constexpr auto total_bits() -> ptrdiff_t
      {
        return bytes * __CHAR_BIT__ + bits;
      }

      auto operator<=>(member_offset const&) const = default;
    };

    //*************************************************************************
    /// Returns the offset of the reflected data member.
    //*************************************************************************
    consteval auto offset_of(info r) -> member_offset
    {
      return member_offset{__metafunction(detail::__metafn_offset_of, ^^ptrdiff_t, r),
                           __metafunction(detail::__metafn_bit_offset_of, ^^ptrdiff_t, r)};
    }

    //*************************************************************************
    /// Returns the size (in bytes) of the reflected type or entity.
    //*************************************************************************
    consteval auto size_of(info r) -> size_t
    {
      return __metafunction(detail::__metafn_size_of, r);
    }

    //*************************************************************************
    /// Returns the size in bits of the reflected type or entity.
    //*************************************************************************
    consteval auto bit_size_of(info r) -> size_t
    {
      return __metafunction(detail::__metafn_bit_size_of, r);
    }

    //*************************************************************************
    /// Returns the alignment of the reflected type or entity.
    //*************************************************************************
    consteval auto alignment_of(info r) -> size_t
    {
      return __metafunction(detail::__metafn_alignment_of, r);
    }

    //*************************************************************************
    /// Returns a reflection of the compile-time constant value.
    //*************************************************************************
    template <typename T>
      requires(!etl::is_reference_v<T> && is_structural_type(^^T))
    consteval auto reflect_constant(T r) -> info
    {
      return __metafunction(detail::__metafn_reflect_result, ^^T, r);
    }

    //*************************************************************************
    /// Returns a reflection of the object designated by the argument.
    //*************************************************************************
    template <typename T>
      requires(!etl::is_function_v<etl::remove_reference_t<T> >)
    consteval auto reflect_object(T& r) -> info
    {
      return __metafunction(detail::__metafn_reflect_result, type_of(^^r), r);
    }

    //*************************************************************************
    /// Returns a reflection of the function designated by the argument.
    //*************************************************************************
    template <typename T>
      requires(etl::is_function_v<etl::remove_reference_t<T> >)
    consteval auto reflect_function(T& r) -> info
    {
      return __metafunction(detail::__metafn_reflect_result, type_of(^^r), r);
    }

        //*************************************************************************
        /// Options for describing a data member (used with data_member_spec).
        /// Requires STL (std::string, std::optional).
        //*************************************************************************
    #if ETL_USING_STL
    struct data_member_options
    {
      struct name_type
      {
        template <typename T>
          requires etl::is_constructible_v<std::string, T>
        consteval name_type(T&& in)
          : s((T&&)in)
        {
        }

        std::string s;
      };

      std::optional<name_type> name              = std::nullopt;
      std::optional<int>       alignment         = std::nullopt;
      std::optional<int>       width             = std::nullopt;
      bool                     no_unique_address = false;
    };

    //*************************************************************************
    /// Returns a reflection representing a description of a data member.
    //*************************************************************************
    consteval auto data_member_spec(info member_type, data_member_options options = {}) -> info
    {
      if (!is_type(member_type))
        throw "'member_type' must represent a type";

      bool has_name          = options.name.has_value();
      int  alignment         = options.alignment.value_or(0);
      int  width             = options.width.value_or(0);
      bool no_unique_address = options.no_unique_address;

      if (width)
      {
        if (alignment)
          throw "Cannot specify both width and alignment for data member";
        if (no_unique_address)
          throw "Cannot specify both width and no_unique_address for data member";
        if (width < 0)
          throw "Cannot specify a negative width for data member";
        if (!is_integral_type(member_type) && !is_enum_type(member_type))
          throw "Bit field must have integral or enumeration type";
      }

      if (alignment)
      {
        if (alignment < int(alignment_of(member_type)))
          throw "Cannot specify an alignment less than that of the member type";
        if (alignment <= 0 || ((alignment - 1) & alignment))
          throw "Alignment specifier must be a non-negative power of 2";
      }

      if (has_name)
      {
        const std::string& s = options.name->s;
        return __metafunction(detail::__metafn_data_member_spec, member_type, !s.empty(), s.size(), ^^const char, s.data(),
                              options.alignment.has_value(), alignment, options.width.has_value(), width, no_unique_address, size_t(0),
                              (const info*)nullptr);
      }
      else
      {
        return __metafunction(detail::__metafn_data_member_spec, member_type, false, 0, ^^const char, "", options.alignment.has_value(), alignment,
                              options.width.has_value(), width, no_unique_address, size_t(0), (const info*)nullptr);
      }
    }

    //*************************************************************************
    /// Completes the definition of the record type with the given members.
    //*************************************************************************
    template <reflection_range R = std::initializer_list<info> >
    consteval auto define_aggregate(info class_type, R&& members) -> info
    {
      std::vector<info> vmembers(members.begin(), members.end());
      return __metafunction(detail::__metafn_define_aggregate, class_type, vmembers.size(), vmembers.data());
    }
    #endif // ETL_USING_STL (data_member_options, data_member_spec, define_aggregate)

    #if !ETL_USING_STL
    //*************************************************************************
    /// Options for describing a data member (non-STL version).
    /// Uses etl::string_view instead of std::string, and sentinel values
    /// instead of std::optional. The API is designed to be compatible with
    /// the STL version's designated-initializer syntax.
    //*************************************************************************
    struct data_member_options
    {
      struct name_type
      {
        consteval name_type() = default;

        consteval name_type(etl::string_view in)
          : sv(in)
          , specified(true)
        {
        }

        consteval name_type(const char* in)
          : sv(in)
          , specified(true)
        {
        }

        etl::string_view sv        = {};
        bool             specified = false;
      };

      name_type name              = {};
      int       alignment         = 0;
      int       width             = 0;
      bool      no_unique_address = false;
    };

    //*************************************************************************
    /// Returns a reflection representing a description of a data member.
    //*************************************************************************
    consteval auto data_member_spec(info member_type, data_member_options options = {}) -> info
    {
      if (!is_type(member_type))
        throw "'member_type' must represent a type";

      bool has_name          = options.name.specified;
      int  alignment         = options.alignment;
      int  width             = options.width;
      bool has_alignment     = alignment != 0;
      bool has_width         = width != 0;
      bool no_unique_address = options.no_unique_address;

      if (has_width)
      {
        if (has_alignment)
          throw "Cannot specify both width and alignment for data member";
        if (no_unique_address)
          throw "Cannot specify both width and no_unique_address for data member";
        if (width < 0)
          throw "Cannot specify a negative width for data member";
        if (!is_integral_type(member_type) && !is_enum_type(member_type))
          throw "Bit field must have integral or enumeration type";
      }

      if (has_alignment)
      {
        if (alignment < int(alignment_of(member_type)))
          throw "Cannot specify an alignment less than that of the member type";
        if (alignment <= 0 || ((alignment - 1) & alignment))
          throw "Alignment specifier must be a non-negative power of 2";
      }

      if (has_name)
      {
        etl::string_view s = options.name.sv;
        return __metafunction(detail::__metafn_data_member_spec, member_type, !s.empty(), s.size(), ^^const char, s.data(), has_alignment, alignment,
                              has_width, width, no_unique_address);
      }
      else
      {
        return __metafunction(detail::__metafn_data_member_spec, member_type, false, 0, ^^const char, "", has_alignment, alignment, has_width, width,
                              no_unique_address);
      }
    }

    //*************************************************************************
    /// Completes the definition of the record type with the given members.
    //*************************************************************************
    template <size_t N>
    consteval auto define_aggregate(info class_type, const info (&members)[N]) -> info
    {
      return __metafunction(detail::__metafn_define_aggregate, class_type, N, &members[0]);
    }

    consteval auto define_aggregate(info class_type, const info_array& members) -> info
    {
      return __metafunction(detail::__metafn_define_aggregate, class_type, members.size(), members.data());
    }
    #endif // !ETL_USING_STL (data_member_options, data_member_spec, define_aggregate)

    //*************************************************************************
    /// Returns whether the visibility of the reflected entity was specified.
    //*************************************************************************
    consteval auto is_access_specified(info r) -> bool
    {
      return __metafunction(detail::__metafn_is_access_specified, r);
    }

        //*************************************************************************
        /// reflect_invoke — requires STL (std::vector, std::initializer_list).
        //*************************************************************************
    #if ETL_USING_STL
    //*************************************************************************
    /// Returns a reflection of the constant value obtained from calling
    /// target(args...).
    //*************************************************************************
    template <reflection_range R = std::initializer_list<info> >
    consteval auto reflect_invoke(info target, R&& args) -> info
    {
      std::vector<info> vargs(args.begin(), args.end());
      return __metafunction(detail::__metafn_reflect_invoke, target, static_cast<info*>(nullptr), 0, vargs.data(), vargs.size());
    }

    template <reflection_range R1 = std::initializer_list<info>, reflection_range R2 = std::initializer_list<info> >
    consteval auto reflect_invoke(info target, R1&& targs, R2&& args) -> info
    {
      std::vector<info> vtargs(targs.begin(), targs.end());
      std::vector<info> vargs(args.begin(), args.end());
      return __metafunction(detail::__metafn_reflect_invoke, target, vtargs.data(), vtargs.size(), vargs.data(), vargs.size());
    }

    #endif // ETL_USING_STL (reflect_invoke)

    #if !ETL_USING_STL
    //*************************************************************************
    /// reflect_invoke (non-STL) — C-array overloads.
    //*************************************************************************

    // reflect_invoke(target, args)
    template <size_t N>
    consteval auto reflect_invoke(info target, const info (&args)[N]) -> info
    {
      return __metafunction(detail::__metafn_reflect_invoke, target, static_cast<info*>(nullptr), 0, &args[0], N);
    }

    consteval auto reflect_invoke(info target, const info_array& args) -> info
    {
      return __metafunction(detail::__metafn_reflect_invoke, target, static_cast<info*>(nullptr), 0, args.data(), args.size());
    }

    // reflect_invoke(target, targs, args)
    template <size_t N1, size_t N2>
    consteval auto reflect_invoke(info target, const info (&targs)[N1], const info (&args)[N2]) -> info
    {
      return __metafunction(detail::__metafn_reflect_invoke, target, &targs[0], N1, &args[0], N2);
    }

    template <size_t N>
    consteval auto reflect_invoke(info target, const info (&targs)[N], const info_array& args) -> info
    {
      return __metafunction(detail::__metafn_reflect_invoke, target, &targs[0], N, args.data(), args.size());
    }

    template <size_t N>
    consteval auto reflect_invoke(info target, const info_array& targs, const info (&args)[N]) -> info
    {
      return __metafunction(detail::__metafn_reflect_invoke, target, targs.data(), targs.size(), &args[0], N);
    }

    consteval auto reflect_invoke(info target, const info_array& targs, const info_array& args) -> info
    {
      return __metafunction(detail::__metafn_reflect_invoke, target, targs.data(), targs.size(), args.data(), args.size());
    }
    #endif // !ETL_USING_STL (reflect_invoke)

    #if defined(__clang__)
    _Pragma("clang diagnostic pop")
    #endif


  } // namespace meta
} // namespace etl

#endif // ETL_PRIVATE_META_CLANG_P2996_H_INCLUDED
