
import VehicleAPI from './vehicleApi.js'

// Clase para gestionar autenticación y sesión
class AuthSystem {


  // Constructor de la clase
  constructor() {
    this.sessionKey = "vehicle_control_session"
  }


  // Verifica si el usuario está autenticado (existe sesión en localStorage)
  isAuthenticated() {
    if (!window.localStorage) return false
    const session = localStorage.getItem(this.sessionKey)
    if (!session) return false
    try {
      const data = JSON.parse(session)
      return !!data.sessionId
    } catch {
      return false
    }
  }


  // Cierra sesión y redirige al login
  logout() {
    if (window.localStorage) {
      localStorage.removeItem(this.sessionKey)
    }
    window.location.href = "index.html"
  }


  // Obtiene la sesión actual
  getSession() {
    if (!window.localStorage) return null
    const session = localStorage.getItem(this.sessionKey)
    if (!session) return null
    try {
      return JSON.parse(session)
    } catch {
      return null
    }
  }
}



// Controlador principal del vehículo (maneja UI y lógica)
class VehicleController {


  // Constructor de la clase
  constructor() {

    // Instancias del sistema de autenticación y API del vehículo
    this.auth = new AuthSystem()
    this.api = new VehicleAPI()

    // Estado inicial de los datos relacionados a los controles
    this.currentDirection = "stop"
    this.currentGear = "N"
    this.gearSpeeds = { 1: 1, 2: 2, 3: 3 }
    this.lightStates = { headlights: false, hazard: false, leftSignal: false, rightSignal: false }

    // Inicializa la app
    this.init()
  }


  // #----- INICIALIZACIÓN DE LA APP -----

  // Función para inicializar la app
  init() {

    // Verifica autenticación, si no hay sesión redirige al login
    if (!this.auth.isAuthenticated()) {
      window.location.href = "index.html"
      return
    }

    // Configura eventos y controles
    this.setupEventListeners()
    this.initializeControls()

    // Inicializa la UI con datos del backend al iniciar
    this.updateStatusFromBackend();

    // Actualiza la UI cada 5 segundos
    setInterval(() => this.updateStatusFromBackend(), 5000);
  }

  // Función para configurar los event listeners de los controles (botones)
  setupEventListeners() {

    // Botón de logout
    document.getElementById("logoutBtn").addEventListener("click", () => {
      this.auth.logout()
    })

    // Botones de dirección (soporte mouse y touch)
    document.querySelectorAll(".dir-btn").forEach((btn) => {

      // Mouse
      btn.addEventListener("mousedown", (e) => this.handleDirectionPress(e))
      btn.addEventListener("mouseup", (e) => this.handleDirectionRelease(e))
      btn.addEventListener("mouseleave", (e) => this.handleDirectionRelease(e))

      // Touch
      btn.addEventListener("touchstart", (e) => {
        e.preventDefault();
        this.handleDirectionPress(e)
      }, { passive: false })

      btn.addEventListener("touchend", (e) => {
        e.preventDefault();
        this.handleDirectionRelease(e)
      }, { passive: false })

      btn.addEventListener("touchcancel", (e) => {
        e.preventDefault();
        this.handleDirectionRelease(e)
      }, { passive: false })
    })

    // Botones de marcha
    document.querySelectorAll(".gear-btn").forEach((btn) => {
      btn.addEventListener("click", (e) => this.handleGearChange(e))
    })

    // Botones de luces
    document.querySelectorAll(".light-btn").forEach((btn) => {
      btn.addEventListener("click", (e) => this.handleLightToggle(e))
    })

  }

  // Función para inicializar controles y UI al cargar (fuerza estado inicial de la marcha en N)
  initializeControls() {
    document.getElementById("gearValue").textContent = this.currentGear
    setTimeout(() => {
      document.querySelectorAll(".gear-btn").forEach((btn) => {
        if (btn.dataset.gear === "N") {
          btn.classList.add("active")
        } else {
          btn.classList.remove("active")
        }
      })
    }, 0)
  }

  // Función para actualizar el estado del vehículo desde el backend y reflejarlo en la UI
  async updateStatusFromBackend() {
    try {
      const status = await this.api.getStatus();

      // Si el backend devuelve datos, actualiza la UI
      if (status) {
        if (status.currentSpeed !== undefined) {
          document.getElementById("currentSpeed").textContent = `${status.currentSpeed} km/h`;
        }
        if (status.movementStatus !== undefined) {
          document.getElementById("movementStatus").textContent = status.movementStatus;
        }
      }
    } catch (error) {
      console.error("Error al actualizar el estado desde el backend:", error);
    }
  }



  // #----- EVENTOS POST AL BACKEND -----

  // Maneja el evento de presionar un botón de dirección
  handleDirectionPress(event) {
    const direction = event.target.dataset.direction
    if (direction) {
      this.setDirection(direction)
      event.target.classList.add("active")

      this.api.sendDirection(direction)
      console.log("Dirección enviada al backend:", direction)
    }
  }

  // Maneja el evento de soltar un botón de dirección
  handleDirectionRelease(event) {
    event.target.classList.remove("active")
    if (this.currentDirection !== "stop") {
      this.setDirection("stop")

      this.api.sendDirection("stop")
      console.log("Dirección enviada al backend:", "stop");
    }
  }

  // Maneja el cambio de marcha (gear)
  handleGearChange(event) {
    const gearValue = event.target.dataset.gear

    if (gearValue === "N") {
      this.currentGear = "N"
      document.getElementById("gearValue").textContent = this.currentGear
      document.querySelectorAll(".gear-btn").forEach((btn) => {
        btn.classList.toggle("active", btn.dataset.gear === "N")
      })

      this.api.setSpeed(0)
      // Cuando está en N, forzar movimiento a "Stopped" y velocidad a 0
      document.getElementById("movementStatus").textContent = "Stopped"
      document.getElementById("currentSpeed").textContent = "0 km/h"
      console.log("Velocidad enviada al backend: 0");
      return
    }

    const gear = parseInt(gearValue)
    if (!gear || gear < 1 || gear > 5) return
    this.currentGear = gear
    document.getElementById("gearValue").textContent = this.currentGear
    document.querySelectorAll(".gear-btn").forEach((btn) => {
      btn.classList.toggle("active", btn.dataset.gear == this.currentGear)
    })

    this.api.setSpeed(this.gearSpeeds[this.currentGear])
    this.updateMovementStatus(this.currentDirection)
    console.log("Velocidad enviada al backend:", this.gearSpeeds[this.currentGear]);
  }

  // Maneja el encendido/apagado de luces
  handleLightToggle(event) {
    const lightType = event.target.dataset.light
    if (lightType) {
      this.lightStates[lightType] = !this.lightStates[lightType]
      event.target.classList.toggle("active", this.lightStates[lightType])

      this.api.toggleLight(lightType, this.lightStates[lightType])
      console.log(`Luz ${lightType} cambiada a:`, this.lightStates[lightType]);
    }
  }



  // #----- ACTUALIZACION Y REFUERZO DE LA UI -----

  // Cambia la dirección del vehículo y actualiza la UI
  setDirection(direction) {
    this.currentDirection = direction
    // Solo actualizar movimiento si la marcha no es N
    if (this.currentGear !== "N") {
      this.updateMovementStatus(direction)
    }

    document.querySelectorAll(".dir-btn").forEach((btn) => {
      btn.classList.remove("active")
    })
    if (direction !== "stop") {
      const activeBtn = document.querySelector(`[data-direction="${direction}"]`)
      if (activeBtn) {
        activeBtn.classList.add("active")
      }
    }
  }

  // Actualiza el estado de movimiento y velocidad en la UI
  updateMovementStatus(direction) {
    const statusElement = document.getElementById("movementStatus")
    const speedElement = document.getElementById("currentSpeed")

    let statusText = "Stopped"
    let speed = 0

    if (direction !== "stop" && this.currentGear !== "N") {
      statusText = direction.charAt(0).toUpperCase() + direction.slice(1)
      // Velocidad según la marcha
      let gearSpeed = this.gearSpeeds[this.currentGear]
      if (typeof gearSpeed !== "number" || isNaN(gearSpeed)) {
        gearSpeed = 0
      }
      speed = Math.round((gearSpeed / 100) * 25) // Convertir a km/h
      if (isNaN(speed)) speed = 0
    }

    statusElement.textContent = statusText
    speedElement.textContent = `${speed} km/h`
  }

}


// Inicializa el controlador del vehículo al cargar la página
const vehicleController = new VehicleController()
