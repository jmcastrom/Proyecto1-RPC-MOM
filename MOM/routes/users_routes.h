#pragma once
#include "mom.grpc.pb.h"
#include "users.grpc.pb.h"
#include <grpcpp/grpcpp.h>

std::string HandleUserRequest(const mom::MOMRequest& request) {
    auto channel = grpc::CreateChannel("54.225.200.222:50053", grpc::InsecureChannelCredentials());
    auto stub = users::UsersService::NewStub(channel);

    if (request.operation() == "Authenticate") {
        users::AuthRequest req;
        req.set_username(request.username());
        req.set_password(request.password());
        users::AuthResponse res;
        grpc::ClientContext ctx;

        auto status = stub->Authenticate(&ctx, req, &res);
        if (!status.ok()) return "Error: " + status.error_message();

        std::ostringstream oss;
        if (res.success()) {
            oss << "Autenticación exitosa:\nUser ID: " << res.user_id() << "\nToken: " << res.token();
        } else {
            oss << "Fallo autenticando: " << res.message();
        }
        return oss.str();
    }

    return "Operación de usuarios no implementada aún";
}

