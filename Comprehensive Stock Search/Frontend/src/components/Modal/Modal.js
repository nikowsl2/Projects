import React from 'react';
import './Modal.css';

const Modal = ({ isOpen, closeModal, children }) => {
    if (!isOpen) return null;

    return (
        <div className="modal-overlay" onClick={closeModal}>
            <div className="modal-content" onClick={e => e.stopPropagation()}>
                {children}
                <button className='modalButton' onClick={closeModal}>X</button>
            </div>
        </div>
    );
};

export default Modal;