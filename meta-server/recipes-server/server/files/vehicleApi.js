class VehicleAPI {
  
  async sendDirection(direction) {
    const response = await fetch('/api/vehicle/direction', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ direction })
    });
    if (!response.ok) {
      throw new Error('Error al enviar la dirección');
    }
    return response.json();
  }

  async setSpeed(speed) {
    const response = await fetch('/api/vehicle/speed', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ speed })
    });
    if (!response.ok) {
      throw new Error('Error al establecer la velocidad');
    }
    return response.json();
  }

  async toggleLight(type, state) {
    const response = await fetch('/api/vehicle/light', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ type, state })
    });
    if (!response.ok) {
      throw new Error('Error al cambiar el estado de la luz');
    }
    return response.json();
  }

  // De momento no usado, se debe de definir que valores pueden venir del backend de estado del vehículo
  // Pueden venir datos en una lista o un objeto con varios parámetros
  async getStatus() {
    const response = await fetch('/api/vehicle/status');
    if (!response.ok) {
      throw new Error('Error al obtener el estado del vehículo');
    }
    return response.json();
  }

  // De momento no usado, se debe de definir como viene el stream de video desde la cámara del vehículo
  async getVideoStream() {
    const response = await fetch('/api/vehicle/video');
    if (!response.ok) {
      throw new Error('Error al obtener el stream de video');
    }
    return response.json();
  }
}

export default VehicleAPI;
