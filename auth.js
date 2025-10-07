class AuthSystem {

  // Constructor de la clase
  constructor() {
  this.sessionKey = "vehicle_control_session"
  this.init()
  }

  // Inicializa el sistema de autenticación
  init() {
    // Verifica si ya hay una sesión activa
    if (this.isAuthenticated()) {
      window.location.href = "control.html"
      return
    }

    const loginForm = document.getElementById("loginForm")
    if (loginForm) {
      loginForm.addEventListener("submit", (e) => this.handleLogin(e))
    }
  }

  // Maneja el evento de envío del formulario de login
  async handleLogin(event) {
    event.preventDefault()

    const username = document.getElementById("username").value
    const password = document.getElementById("password").value
    const errorMessage = document.getElementById("errorMessage")
    const loginBtn = document.querySelector(".login-btn")
    const btnText = document.querySelector(".btn-text")
    const btnLoading = document.querySelector(".btn-loading")

    // Muestra indicador de cargando
    loginBtn.disabled = true
    btnText.style.display = "none"
    btnLoading.style.display = "inline"
    errorMessage.style.display = "none"

    try {
      const resp = await fetch("/api/login", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password }),
      })
      const data = await resp.json()
      if (resp.ok && data.token) {
        // Crea la sesión
        const sessionData = {
          username: username,
          loginTime: new Date().toISOString(),
          sessionId: data.token,
        }
        localStorage.setItem(this.sessionKey, JSON.stringify(sessionData))
        window.location.href = "control.html"
      } else {
        throw new Error(data.error || "Invalid username or password")
      }
    } catch (e) {
      errorMessage.textContent = e.message
      errorMessage.style.display = "block"
      loginBtn.disabled = false
      btnText.style.display = "inline"
      btnLoading.style.display = "none"
    }
  }



  // Genera un ID de sesión único
  generateSessionId() {
    return Math.random().toString(36).substring(2) + Date.now().toString(36)
  }

  // Verifica si la sesión es válida (no expirada)
  isAuthenticated() {
    const session = localStorage.getItem(this.sessionKey)
    if (!session) return false

    try {
      const sessionData = JSON.parse(session)
      const loginTime = new Date(sessionData.loginTime)
      const now = new Date()
      const hoursDiff = (now - loginTime) / (1000 * 60 * 60)

      // Session expires after 8 hours
      return hoursDiff < 8
    } catch (e) {
      return false
    }
  }

  // Cierra sesión y redirige al login
  logout() {
    localStorage.removeItem(this.sessionKey)
    window.location.href = "index.html"
  }

  // Obtiene la sesión actual
  getSession() {
    const session = localStorage.getItem(this.sessionKey)
    return session ? JSON.parse(session) : null
  }
}

// Inicializa la autenticación
const auth = new AuthSystem()
