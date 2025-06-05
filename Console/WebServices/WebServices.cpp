//
// Created by Brett King on 5/6/2025.
//

#include "WebServices.h"

#include <iostream>

WebServices::WebServices()
  {
    std::cout << "WebServices::WebServices()" << std::endl;
    initialize();
  }

WebServices::~WebServices()
  {
    shutdown();
    std::cout << "WebServices::~WebServices()" << std::endl;
  }

void WebServices::initialize()
  {
    std::cout << "WebServices::initialize()" << std::endl;
  }

void WebServices::shutdown()
  {
    std::cout << "WebServices::shutdown()" << std::endl;
  }

void WebServices::handleRequest(const std::string& request)
  {
    std::cout << "WebServices::handleRequest() - Request: " << request << std::endl;
  }