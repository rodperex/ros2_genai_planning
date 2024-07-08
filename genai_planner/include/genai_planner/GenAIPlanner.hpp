// Copyright 2024 Rodrigo Pérez-Rodríguez
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rclcpp/rclcpp.hpp"
#include "ros2openai_interfaces/srv/open_ai_prompt.hpp"
#include "knowledge_graph_interfaces/srv/operate.hpp"
#include "knowledge_graph_interfaces/srv/verify.hpp"
#include "knowledge_graph_interfaces/srv/dump_graph.hpp"
#include "genai_planning_interfaces/srv/plan.hpp"
#include <iostream>
#include <fstream>

#ifndef GENAI_PLANNER_HPP_
#define GENAI_PLANNER_HPP_

namespace genai_planning
{

class GenAIPlanner : public rclcpp::Node
{
public:
  GenAIPlanner();

private:
  void plan(
    const std::shared_ptr<genai_planning_interfaces::srv::Plan::Request> request,
    std::shared_ptr<genai_planning_interfaces::srv::Plan::Response> response);
  bool prompt_genai(const std::string& text, std::string& response);
  bool load_text_file(const std::string& file_path, std::string& text);
  bool collapse_kg(std::string& node, std::string& kg);

  rclcpp::Client<ros2openai_interfaces::srv::OpenAIPrompt>::SharedPtr openai_client_;
  rclcpp::Client<knowledge_graph_interfaces::srv::Operate>::SharedPtr kg_operate_client_;
  rclcpp::Client<knowledge_graph_interfaces::srv::Verify>::SharedPtr kg_verify_client_;
  rclcpp::Client<knowledge_graph_interfaces::srv::DumpGraph>::SharedPtr kg_dump_client_;
  rclcpp::Service<genai_planning_interfaces::srv::Plan>::SharedPtr plan_srv_;

  std::string intro_file_path_ = "ground_floor",
    current_floor_ = "prompts/intro.yaml",
    kg_;
};

}  // namespace genai_planning

#endif // GENAI_PLANNER_HPP_