//---------------------------------------------------------------------------//
// Copyright (c) 2023 Alexey Yashunsky <a.yashunsky@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//
// @file Declaration of interfaces for linear interpolation coefficients component.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_LINEAR_INTER_COEFS_HPP
#define CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_LINEAR_INTER_COEFS_HPP

#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint_system.hpp>

#include <nil/blueprint/blueprint/plonk/assignment.hpp>
#include <nil/blueprint/blueprint/plonk/circuit.hpp>
#include <nil/blueprint/component.hpp>
#include <nil/blueprint/manifest.hpp>

namespace nil {
    namespace blueprint {
        namespace components {
            // linear interpolation with points (x0,z0), (x1,z1)
            // Input: x0, z0, x1, z1
            // Output: a0, a1, such that the line z = a0 + a1*x passes through both points
            // checks that x0 != x1 (otherwise a constraint is violated)
            template<typename ArithmetizationType, typename BlueprintFieldType>
            class linear_inter_coefs;

            template<typename BlueprintFieldType>
            class linear_inter_coefs<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>,
                           BlueprintFieldType>
                : public plonk_component<BlueprintFieldType> {

            public:
                using component_type = plonk_component<BlueprintFieldType>;

                using var = typename component_type::var;
                using manifest_type = plonk_component_manifest;

                class gate_manifest_type : public component_gate_manifest {
                public:
                    std::uint32_t gates_amount() const override {
                        return linear_inter_coefs::gates_amount;
                    }
                };

                static gate_manifest get_gate_manifest(std::size_t witness_amount,
                                                       std::size_t lookup_column_amount) {
                    static gate_manifest manifest = gate_manifest(gate_manifest_type());
                    return manifest;
                }

                static manifest_type get_manifest() {
                    static manifest_type manifest = manifest_type(
                        std::shared_ptr<manifest_param>(new manifest_single_value_param(7)),
                        false
                    );
                    return manifest;
                }

                constexpr static std::size_t get_rows_amount(std::size_t witness_amount,
                                                             std::size_t lookup_column_amount) {
                    return 1;
                }

                constexpr static const std::size_t gates_amount = 1;
                const std::size_t rows_amount = get_rows_amount(this->witness_amount(), 0);

                struct input_type {
                    var x0, z0, x1, z1;

                    std::vector<std::reference_wrapper<var>> all_vars() {
                        return {x0, z0, x1, z1};
                    }
                };

                struct result_type {
		    std::array<var,2> output;

                    result_type(const linear_inter_coefs &component, std::uint32_t start_row_index) {
                        output = { var(component.W(4), start_row_index, false, var::column_type::witness),
                                   var(component.W(5), start_row_index, false, var::column_type::witness)};
                    }

                    std::vector<var> all_vars() const {
                        return {output[0], output[1]};
                    }
                };

                template<typename ContainerType>
                explicit linear_inter_coefs(ContainerType witness) : component_type(witness, {}, {}, get_manifest()) {};

                template<typename WitnessContainerType, typename ConstantContainerType,
                         typename PublicInputContainerType>
                linear_inter_coefs(WitnessContainerType witness, ConstantContainerType constant,
                         PublicInputContainerType public_input) :
                    component_type(witness, constant, public_input, get_manifest()) {};

                linear_inter_coefs(
                    std::initializer_list<typename component_type::witness_container_type::value_type>
                        witnesses,
                    std::initializer_list<typename component_type::constant_container_type::value_type>
                        constants,
                    std::initializer_list<typename component_type::public_input_container_type::value_type>
                        public_inputs) :
                    component_type(witnesses, constants, public_inputs, get_manifest()) {};
            };

            template<typename BlueprintFieldType>
            using plonk_linear_inter_coefs =
                linear_inter_coefs<
                    crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>,
                    BlueprintFieldType>;

            template<typename BlueprintFieldType>
            typename plonk_linear_inter_coefs<BlueprintFieldType>::result_type generate_assignments(
                const plonk_linear_inter_coefs<BlueprintFieldType> &component,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>>
                    &assignment,
                const typename plonk_linear_inter_coefs<BlueprintFieldType>::input_type
                    &instance_input,
                const std::uint32_t start_row_index) {

                using value_type = typename BlueprintFieldType::value_type;
                value_type x0 = var_value(assignment, instance_input.x0),
                           z0 = var_value(assignment, instance_input.z0),
                           x1 = var_value(assignment, instance_input.x1),
                           z1 = var_value(assignment, instance_input.z1);

                assignment.witness(component.W(0), start_row_index) = x0;
                assignment.witness(component.W(1), start_row_index) = z0;
                assignment.witness(component.W(2), start_row_index) = x1;
                assignment.witness(component.W(3), start_row_index) = z1;
                if (x0 != x1) { // normal case
                    assignment.witness(component.W(4), start_row_index) = (x1*z0 - x0*z1) / (x1-x0);
                    assignment.witness(component.W(5), start_row_index) = (z1-z0) / (x1-x0);
                    assignment.witness(component.W(6), start_row_index) = 1 / (x1-x0);
                } else { // just make some assignments that will fail
                    assignment.witness(component.W(4), start_row_index) = 0;
                    assignment.witness(component.W(5), start_row_index) = 0;
                    assignment.witness(component.W(6), start_row_index) = 0;
                }
                return typename plonk_linear_inter_coefs<BlueprintFieldType>::result_type(
                    component, start_row_index);
	    }

            template<typename BlueprintFieldType>
            std::size_t generate_gates(
                const plonk_linear_inter_coefs<BlueprintFieldType> &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>> &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>>
                    &assignment,
                const typename plonk_linear_inter_coefs<BlueprintFieldType>::input_type
                    &instance_input) {

                using var = typename plonk_linear_inter_coefs<BlueprintFieldType>::var;
                var X0 = var(component.W(0), 0, true),
                    Z0 = var(component.W(1), 0, true),
                    X1 = var(component.W(2), 0, true),
                    Z1 = var(component.W(3), 0, true),
                    A0 = var(component.W(4), 0, true),
                    A1 = var(component.W(5), 0, true),
                    I  = var(component.W(6), 0, true);

                return bp.add_gate({A1*X0 + A0 - Z0, A1*X1 + A0 - Z1, (X1 - X0)*I - 1});
            }

            template<typename BlueprintFieldType>
            void generate_copy_constraints(
                const plonk_linear_inter_coefs<BlueprintFieldType> &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>> &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>>
                    &assignment,
                const typename plonk_linear_inter_coefs<BlueprintFieldType>::input_type &instance_input,
                const std::size_t start_row_index) {

                using var = typename plonk_linear_inter_coefs<BlueprintFieldType>::var;

                bp.add_copy_constraint({var(component.W(0), start_row_index, false), instance_input.x0});
                bp.add_copy_constraint({var(component.W(1), start_row_index, false), instance_input.z0});
                bp.add_copy_constraint({var(component.W(2), start_row_index, false), instance_input.x1});
                bp.add_copy_constraint({var(component.W(3), start_row_index, false), instance_input.z1});
            }

            template<typename BlueprintFieldType>
            typename plonk_linear_inter_coefs<BlueprintFieldType>::result_type generate_circuit(
                const plonk_linear_inter_coefs<BlueprintFieldType> &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>> &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType>>
                    &assignment,
                const typename plonk_linear_inter_coefs<BlueprintFieldType>::input_type &instance_input,
                const std::size_t start_row_index) {

                std::size_t selector_index = generate_gates(component, bp, assignment, instance_input);
                assignment.enable_selector(selector_index, start_row_index);
                generate_copy_constraints(component, bp, assignment, instance_input, start_row_index);

                return typename plonk_linear_inter_coefs<BlueprintFieldType>::result_type(
                    component, start_row_index);
            }

        }    // namespace components
    }        // namespace blueprint
}    // namespace nil

#endif    // CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_LINEAR_INTER_COEFS_HPP
