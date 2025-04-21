// client.go - Ejemplo de cliente para probar el servicio
package main

import (
	"context"
	"log"
	"time"

	pb "libreria/users"

	"google.golang.org/grpc"
)

func main() {
	// Establecer conexión con el servidor
	conn, err := grpc.Dial("localhost:50053", grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()
	client := pb.NewUsersServiceClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	// Registrar un nuevo usuario
	registerResp, err := client.Register(ctx, &pb.RegisterRequest{
		Username: "new_user",
		Email:    "new@example.com",
		Password: "securepass",
		FullName: "New User",
		Address:  "789 Oak St, Newtown",
	})
	if err != nil {
		log.Fatalf("could not register: %v", err)
	}
	log.Printf("User registered: %s", registerResp.UserId)

	// Autenticar usuario
	authResp, err := client.Authenticate(ctx, &pb.AuthRequest{
		Username: "new_user",
		Password: "securepass",
	})
	if err != nil {
		log.Fatalf("could not authenticate: %v", err)
	}
	log.Printf("Authentication result: %v, Token: %s", authResp.Success, authResp.Token)

	// Obtener información del usuario
	getUserResp, err := client.GetUser(ctx, &pb.GetUserRequest{
		UserId: registerResp.UserId,
	})
	if err != nil {
		log.Fatalf("could not get user: %v", err)
	}
	log.Printf("User info: %s, %s, %s", getUserResp.Username, getUserResp.Email, getUserResp.FullName)

	// Actualizar información del usuario
	updateResp, err := client.UpdateUser(ctx, &pb.UpdateUserRequest{
		UserId:   registerResp.UserId,
		Email:    "updated@example.com",
		FullName: "Updated User Name",
		Address:  "Updated Address",
	})
	if err != nil {
		log.Fatalf("could not update user: %v", err)
	}
	log.Printf("Updated user: %s, %s, %s", updateResp.Username, updateResp.Email, updateResp.FullName)
}
