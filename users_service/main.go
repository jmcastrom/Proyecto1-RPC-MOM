// main.go
package main

import (
	"encoding/json"
    	"os"
	"context"
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"log"
	"math/rand"
	"net"
	"sync"
	"time"

	orders_pb "libreria/orders"
	pb "libreria/users"

	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const (
	port = ":50053"
)

type User struct {
	UserID    string
	Username  string
	Email     string
	FullName  string
	Address   string
	Password  string // En una aplicación real, esto sería un hash
	CreatedAt string
}

type UsersServer struct {
	pb.UnimplementedUsersServiceServer
	users        map[string]*User
	usersByName  map[string]string
	usersByEmail map[string]string
	mu           sync.Mutex
	ordersClient orders_pb.OrdersServiceClient
}

var (
    usersMap = make(map[string]User)
    usersMutex sync.Mutex
)

func (s *UsersServer) LoadFromFile(filename string) error {
	file, err := os.Open(filename)
	if err != nil {
		if os.IsNotExist(err) {
			return nil // Si no existe, lo ignoramos
		}
		return err
	}
	defer file.Close()

	var loadedUsers map[string]*User
	decoder := json.NewDecoder(file)
	if err := decoder.Decode(&loadedUsers); err != nil {
		return err
	}

	// Bloqueamos antes de modificar el estado
	//s.mutex.Lock()
	s.mu.Lock()
	//defer s.mutex.Unlock()
	defer s.mu.Unlock()

	s.users = loadedUsers
	s.usersByName = make(map[string]string)
	s.usersByEmail = make(map[string]string)

	for id, user := range loadedUsers {
		s.usersByName[user.Username] = id
		s.usersByEmail[user.Email] = id
	}

	return nil
}

func (s *UsersServer) SaveToFile(filename string) error {
    s.mu.Lock()
    defer s.mu.Unlock()

    log.Println("🔁 Guardando usuarios en JSON...")

    data := make(map[string]*User)
    for id, user := range s.users {
        data[id] = user
    }

    file, err := os.Create(filename)
    if err != nil {
        log.Printf("❌ Error al crear archivo JSON: %v", err)
        return err
    }
    defer file.Close()

    encoder := json.NewEncoder(file)
    encoder.SetIndent("", "  ")
    err = encoder.Encode(data)

    if err != nil {
        log.Printf("❌ Error al codificar JSON: %v", err)
    } else {
        log.Println("✅ Usuarios guardados exitosamente en", filename)
    }

    return err
}


func generateUserID() string {
	const charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	b := make([]byte, 8)
	for i := range b {
		b[i] = charset[rand.Intn(len(charset))]
	}
	return string(b)
}

func getCurrentTime() string {
	return time.Now().Format("2006-01-02 15:04:05")
}

func hashPassword(password string) string {
	hash := sha256.Sum256([]byte(password))
	return hex.EncodeToString(hash[:])
}

func NewUsersServer(ordersClient orders_pb.OrdersServiceClient) *UsersServer {
	return &UsersServer{
		users:        make(map[string]*User),
		usersByName:  make(map[string]string),
		usersByEmail: make(map[string]string),
		ordersClient: ordersClient,
	}
}

func (s *UsersServer) Register(ctx context.Context, req *pb.RegisterRequest) (*pb.UserResponse, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	log.Println("🧑‍💻 [UsersService] Register llama username:", req.Username)
	// Verificar si el usuario o el email ya existen
	if _, exists := s.usersByName[req.Username]; exists {
		return nil, status.Errorf(codes.AlreadyExists, "El nombre de usuario ya está en uso")
	}

	if _, exists := s.usersByEmail[req.Email]; exists {
		return nil, status.Errorf(codes.AlreadyExists, "El email ya está registrado")
	}
	log.Println("✅ Enviand2o respuesta al MOM con userID:",  req.Username)

	// Crear nuevo usuario
	userID := generateUserID()
	newUser := &User{
		UserID:    userID,
		Username:  req.Username,
		Email:     req.Email,
		FullName:  req.FullName,
		Address:   req.Address,
		Password:  hashPassword(req.Password),
		CreatedAt: getCurrentTime(),
	}

	log.Println("✅ Enviado1 respuesta al MOM con userID:", req.Username)


	// Guardar el usuario
	s.users[userID] = newUser
	s.usersByName[req.Username] = userID
	s.usersByEmail[req.Email] = userID

	log.Println("✅ Enviando respuesta al MOM con userID:", req.Username)
	

	//  Guardar en archivo JSON, pero sin bloquear la ejecución
	go func() {
	    defer func() {
	        if r := recover(); r != nil {
	            log.Printf("⚠️ Panic al guardar usuarios.json: %v", r)
	        }
	    }()

	    if err := s.SaveToFile("users.json"); err != nil {
	        log.Printf("❌ Error al guardar usuarios.json: %v", err)
	    } else {
	        log.Println("✅ Guardado exitoso en users.json")
	    }
	}()
		
	/*  Guardar en archivo JSON
	if err := s.SaveToFile("users.json"); err != nil {
    		log.Printf("Error al guardar usuarios: %v", err)
	}*/
	//_ = s.SaveToFile("users.json")
	//_ = s.SaveToFile("/home/ubuntu/users/library/users.json")

	log.Printf("✅ Registro completo, enviando respuesta para userID: %s", userID)
	// Preparar la respuesta
	return &pb.UserResponse{
		UserId:    userID,
		Username:  newUser.Username,
		Email:     newUser.Email,
		FullName:  newUser.FullName,
		Address:   newUser.Address,
		CreatedAt: newUser.CreatedAt,
	}, nil
}

func (s *UsersServer) GetUser(ctx context.Context, req *pb.GetUserRequest) (*pb.UserResponse, error) {
	log.Println("🧑‍💻 [UsersServiceGetUser llamado user id:", req.GetUserId())
	s.mu.Lock()
	defer s.mu.Unlock()

	user, exists := s.users[req.UserId]
	if !exists {
		return nil, status.Errorf(codes.NotFound, "Usuario no encontrado")
	}

	return &pb.UserResponse{
		UserId:    user.UserID,
		Username:  user.Username,
		Email:     user.Email,
		FullName:  user.FullName,
		Address:   user.Address,
		CreatedAt: user.CreatedAt,
	}, nil
}

func (s *UsersServer) UpdateUser(ctx context.Context, req *pb.UpdateUserRequest) (*pb.UserResponse, error) {
	log.Println("✏️ [UsersService] UpdateUser llamado - user_id:", req.UserId)
	s.mu.Lock()
	defer s.mu.Unlock()

	user, exists := s.users[req.UserId]
	if !exists {
		return nil, status.Errorf(codes.NotFound, "Usuario no encontrado")
	}

	// Actualizar el email si se proporciona uno nuevo y es diferente
	if req.Email != "" && req.Email != user.Email {
		if _, exists := s.usersByEmail[req.Email]; exists {
			return nil, status.Errorf(codes.AlreadyExists, "El email ya está registrado")
		}
		// Eliminar el email antiguo del mapa
		delete(s.usersByEmail, user.Email)
		// Actualizar el email
		user.Email = req.Email
		// Registrar el nuevo email
		s.usersByEmail[req.Email] = user.UserID
	}

	// Actualizar otros campos si se proporcionan
	if req.FullName != "" {
		user.FullName = req.FullName
	}
	if req.Address != "" {
		user.Address = req.Address
	}

	 //  Guardar en archivo JSON, pero sin bloquear la ejecución
        go func() {
            defer func() {
                if r := recover(); r != nil {
                    log.Printf("⚠️ Panic al guardar usuarios.json: %v", r)
                }
            }()

            if err := s.SaveToFile("users.json"); err != nil {
                log.Printf("❌ Error al guardar usuarios.json: %v", err)
            } else {
                log.Println("✅ Guardado exitoso en users.json")
            }
        }()
	
	/* Guardar cambios en archivo
   	//_ = s.SaveToFile("users.json")
	//_ = s.SaveToFile("/home/ubuntu/users/library/users.json")
	if err := s.SaveToFile("users.json"); err != nil {
    		log.Printf("Error al guardar usuarios: %v", err)
	}*/


	return &pb.UserResponse{
		UserId:    user.UserID,
		Username:  user.Username,
		Email:     user.Email,
		FullName:  user.FullName,
		Address:   user.Address,
		CreatedAt: user.CreatedAt,
	}, nil
}

func (s *UsersServer) Authenticate(ctx context.Context, req *pb.AuthRequest) (*pb.AuthResponse, error) {
	log.Println("🧑‍💻 [UsersServic Authenticate llamado - username:", req.GetUsername())
	s.mu.Lock()
	defer s.mu.Unlock()

	userID, exists := s.usersByName[req.Username]
	if !exists {
		return &pb.AuthResponse{
			Success: false,
			Message: "Usuario o contraseña incorrectos",
		}, nil
	}

	user := s.users[userID]
	hashedPassword := hashPassword(req.Password)

	if user.Password != hashedPassword {
		return &pb.AuthResponse{
			Success: false,
			Message: "Usuario o contraseña incorrectos",
		}, nil
	}

	// En una aplicación real, aquí se generaría un JWT o token de sesión
	// Para este ejemplo, simplemente devolvemos un token simulado
	token := fmt.Sprintf("token-%s-%d", userID, time.Now().Unix())

	return &pb.AuthResponse{
		Success: true,
		UserId:  userID,
		Token:   token,
		Message: "Autenticación exitosa",
	}, nil
}

// GetUserOrders obtiene los pedidos de un usuario utilizando el servicio de pedidos
func (s *UsersServer) GetUserOrders(ctx context.Context, userID string, page, pageSize int32) (*orders_pb.ListOrdersResponse, error) {
	// Verificar si el usuario existe
	s.mu.Lock()
	_, exists := s.users[userID]
	s.mu.Unlock()

	if !exists {
		return nil, status.Errorf(codes.NotFound, "Usuario no encontrado")
	}

	// Llamar al servicio de pedidos
	req := &orders_pb.ListOrdersRequest{
		UserId:   userID,
		Page:     page,
		PageSize: pageSize,
	}

	return s.ordersClient.ListOrdersByUser(ctx, req)
}

func main() {
	rand.Seed(time.Now().UnixNano())

	lis, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	// Crear conexión con el servicio de pedidos
	ordersConn, err := grpc.Dial("localhost:50052", grpc.WithInsecure())
	if err != nil {
		log.Fatalf("Failed to connect to orders service: %v", err)
	}
	defer ordersConn.Close()
	ordersClient := orders_pb.NewOrdersServiceClient(ordersConn)

	// Crear servidor gRPC y el servicio de usuarios
	server := grpc.NewServer()
	usersServer := NewUsersServer(ordersClient)

	//  Cargar usuarios desde JSON
	err = usersServer.LoadFromFile("users.json")
	if err != nil {
		log.Printf("Error al cargar usuarios desde archivo: %v", err)
	}

	/* Añadir algunos usuarios de ejemplo para pruebas
	usersServer.users["U12345"] = &User{
		UserID:    "U12345",
		Username:  "johndoe",
		Email:     "john@example.com",
		FullName:  "John Doe",
		Address:   "123 Main St, Anytown",
		Password:  hashPassword("password123"),
		CreatedAt: "2023-01-01 10:00:00",
	}
	usersServer.usersByName["johndoe"] = "U12345"
	usersServer.usersByEmail["john@example.com"] = "U12345"

	usersServer.users["U67890"] = &User{
		UserID:    "U67890",
		Username:  "janedoe",
		Email:     "jane@example.com",
		FullName:  "Jane Doe",
		Address:   "456 Elm St, Othertown",
		Password:  hashPassword("password456"),
		CreatedAt: "2023-01-02 11:00:00",
	}
	usersServer.usersByName["janedoe"] = "U67890"
	usersServer.usersByEmail["jane@example.com"] = "U67890"
	*/

	// Registrar el servicio solo una vez
	pb.RegisterUsersServiceServer(server, usersServer)

	log.Printf("Users Service listening on %s", port)
	if err := server.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
