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

#include "genai_planner/GenAIPlanner.hpp"

namespace genai_planning
{

GenAIPlanner::GenAIPlanner()
: Node("genai_planner")
{
  openai_client_ = this->create_client<ros2openai_interfaces::srv::OpenAIPrompt>("openai_prompt");
  kg_operate_client_ = this->create_client<knowledge_graph_interfaces::srv::Operate>("kg_operate");
  kg_verify_client_ = this->create_client<knowledge_graph_interfaces::srv::Verify>("kg_verify");
  kg_dump_client_ = this->create_client<knowledge_graph_interfaces::srv::DumpGraph>("kg_dump");

  plan_srv_ = this->create_service<genai_planning_interfaces::srv::Plan>("plan",
    std::bind(&GenAIPlanner::plan, this, std::placeholders::_1, std::placeholders::_2));

  declare_parameter("intro_file_path", intro_file_path_);
  declare_parameter("current_floor", current_floor_);
}

void
GenAIPlanner::plan(
  const std::shared_ptr<genai_planning_interfaces::srv::Plan::Request> request,
  std::shared_ptr<genai_planning_interfaces::srv::Plan::Response> response)
{
  std::string prompt, genai_answ;

  // load intro text
  if (!load_text_file(intro_file_path_, prompt))
  {
    RCLCPP_ERROR(this->get_logger(), "Failed to load intro prompt");
    response->success = false;
    return;
  }
  
  // prompt the AI with the intro text
  if (!prompt_genai(prompt, genai_answ))
  {
    RCLCPP_ERROR(this->get_logger(), "Failed to prompt the AI");
    response->success = false;
    return;
  }

  // get the knowledge graph collapsed around the current floor
  if (!collapse_kg(current_floor_, kg_))
  {
    RCLCPP_ERROR(this->get_logger(), "Failed to collapse the knowledge graph");
    response->success = false;
    return;
  }

  // prompt the AI with the collapsed knowledge graph
  if (!prompt_genai(kg_, genai_answ))
  {
    RCLCPP_ERROR(this->get_logger(), "Failed to prompt the AI");
    response->success = false;
    return;
  }

  // prompt the AI with the goal and instructions
  prompt = "goal: " + request->goal;
  if (!prompt_genai(request->goal, genai_answ))
  {
    RCLCPP_ERROR(this->get_logger(), "Failed to prompt the AI");
    response->success = false;
    return;
  }

  // process answer and continue planning

  response->success = true;
}

bool
GenAIPlanner::prompt_genai(const std::string& text, std::string& response)
{
  // wait for the service to be available
  while (!openai_client_->wait_for_service(std::chrono::seconds(1))) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
      return false;
    }
    RCLCPP_INFO(this->get_logger(), "Service not available. Retrying...");
  }

  auto request = std::make_shared<ros2openai_interfaces::srv::OpenAIPrompt::Request>();
  request->prompt = text;

  auto future = openai_client_->async_send_request(request);
  if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) ==
    rclcpp::FutureReturnCode::SUCCESS)
  {
    auto result = future.get();
    response = result->message;
    return result->success;
  }
  else
  {
    return false;
  }
}

bool
GenAIPlanner::load_text_file(const std::string& file_path, std::string& text){
  std::ifstream file(file_path);

  if (!file.is_open())
  {
    RCLCPP_ERROR(this->get_logger(), "Failed to open file: %s", file_path.c_str());
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();

  text =  buffer.str();
  return true;
}

bool
GenAIPlanner::collapse_kg(std::string& node, std::string& kg)
{
  auto request = std::make_shared<knowledge_graph_interfaces::srv::Operate::Request>();
  request->operation = "collapse";
  request->payload[0] = node;

  auto future = kg_operate_client_->async_send_request(request);
  if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) ==
    rclcpp::FutureReturnCode::SUCCESS)
  {
    auto result = future.get();
    kg = result->kg_str;
    return result->success;
  }
  else
  {
    return false;
  }
}

} // namespace genai_planning