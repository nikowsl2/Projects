import React, { useState } from 'react';
import Modal from '../Modal/Modal';
import './NewsInfo.css';
import { FacebookShareButton, FacebookIcon } from 'react-share';
import { TwitterShareButton, XIcon } from 'react-share';
import { Container, Row, Col } from 'react-bootstrap';


const NewsInfo = ({ companyData }) => {
    const [isModalOpen, setIsModalOpen] = useState(false);
    const [selectedNewsItem, setSelectedNewsItem] = useState(null);
    const newsSet = companyData.newsSet || [];
    const completeNewsItems = newsSet.filter(newsItem =>
        newsItem.Image && newsItem.Title && newsItem.Description &&
        newsItem.LinkToOriginalPost && newsItem.Source && newsItem.PublishedDate
    );
    const validNewsItems = completeNewsItems.slice(0, 20);

    const openModal = (newsItem) => {
        setSelectedNewsItem(newsItem);
        setIsModalOpen(true);
    };

    const formatDate = (timestamp) => {
        const date = new Date(timestamp * 1000);
        const options = { year: 'numeric', month: 'long', day: 'numeric' };
        return date.toLocaleDateString('en-US', options);
    };

    return (
        <div className="news-container">
            <Container>
                <Row>
                    {validNewsItems.map((item, index) => (
                        <Col xs={12} md={6} key={index} onClick={() => openModal(item)} className="mb-4">
                            <div className="news-item-container">
                                <Row>
                                    <Col xs={12} lg={6} className="d-flex justify-content-center">
                                        <img src={item.Image} alt={item.Title} className="news-image" style={{ width: '100%', maxHeight: '200px', objectFit: 'cover' }} />
                                    </Col>
                                    <Col xs={12} lg={6} className="d-flex align-items-center">
                                        <div>
                                            <div className="news-title">{item.Title}</div>
                                        </div>
                                    </Col>
                                </Row>
                            </div>
                        </Col>
                    ))}
                </Row>
            </Container>

            <Modal isOpen={isModalOpen} closeModal={() => setIsModalOpen(false)}>
                {selectedNewsItem && (
                    <div>
                        <h2 className='ModalSource'>{selectedNewsItem.Source}</h2>
                        <p className='ModalDate'>{formatDate(selectedNewsItem.PublishedDate)}</p>
                        <div className="modal-divider"></div>
                        <p className='ModalTitle'>{selectedNewsItem.Title}</p>
                        <p className='ModalDescription'>{selectedNewsItem.Description}</p>
                        <p className='ForMoreDetails'>For more details click <a href={selectedNewsItem.LinkToOriginalPost} target="_blank" rel="noopener noreferrer">here</a></p>
                        <div className='socialContainer'>
                            <p>Share</p>
                            <div className="socialButtons">
                                <TwitterShareButton
                                    title={selectedNewsItem.Title}
                                    url={selectedNewsItem.LinkToOriginalPost}>
                                    <XIcon size={32} />
                                </TwitterShareButton>
                                <FacebookShareButton
                                    url={selectedNewsItem.LinkToOriginalPost}
                                    quote={selectedNewsItem.Title}>
                                    <FacebookIcon size={32} />
                                </FacebookShareButton>
                            </div>
                        </div>
                    </div>
                )}
            </Modal>
        </div>
    );
};

export default NewsInfo;
