<?php

/*
 * @file
 * Contains PrpInstagramFeedFetcher and related classes.
 */

/**
 * Result of FeedsHTTPFetcher::fetch().
 */
class PrpInstagramFeedFetcherResult extends FeedsFetcherResult {

  protected $access_token;

  protected $client_secret;

  protected $username;

  protected $file_path;

  /**
   * Constructor.
   */
  public function __construct($client_id) {
    $this->client_id = $client_id;

    try {
      $access_token = $this->getAccessToken($client_id);
      $this->access_token = $access_token;
    }
    catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      prp_instagram_feed_watchdog_custom('Instagram Feeds', $e->getMessage(), NULL, WATCHDOG_ERROR);
    }

    $this->url = 'https://api.instagram.com/v1/users/self/media/recent?access_token=' . $access_token;
    parent::__construct('');
  }

  /**
   * Overrides FeedsFetcherResult::getRaw();
   */
  public function getRaw() {
    feeds_include_library('http_request.inc', 'http_request');
    $result = http_request_get($this->url);

    if (!in_array($result->code, array(200, 201, 202, 203, 204, 205, 206))) {
      throw new Exception(t('Download of @url failed with code !code.', array('@url' => $this->url, '!code' => $result->code)));
    };

    return $this->sanitizeRaw($result->data);
  }

  protected function getAccessToken($client_id) {
    $result = db_select('prp_instagram_feed_oauth', 'pifo')
      ->fields('pifo', array('oauth_token'))
      ->condition('oauth_client_id', $client_id)
      ->execute()
      ->fetchAssoc();

    if (isset($result['oauth_token']) && !empty($result['oauth_token'])) {
      return $result['oauth_token'];
    }
    else {
      throw new Exception(t('There is no generate access token for this client id, please go to @link to generate one', array('@link' => 'admin/config/services/instagram-parser')));
    }
  }

}

class PrpInstagramFeedFetcher extends FeedsHTTPFetcher {

  /**
   * Implements FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);

    return new PrpInstagramFeedFetcherResult($source_config['prp_instagram_feed_oauth_client_id']);
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = array();

    $form['no_config_settings'] = array(
      '#type' => 'markup',
      '#markup' => t('There are no config settings for this fetcher.'),
    );

    return $form;
  }

  /**
   * Override sourceSave() - subscribe to hub.
   */
  public function sourceSave(FeedsSource $source) {
  }

  /**
   * Expose source form.
   */
  public function sourceForm($source_config) {
    $form = array();

    $form['prp_instagram_feed_oauth_client_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Instagram Client ID'),
      '#default_value' => isset($source_config['prp_instagram_feed_oauth_client_id']) ? $source_config['prp_instagram_feed_oauth_client_id'] : '',
      '#description' => t('To generate access token go to') . ' ' . l(t('Generate Access Token'), 'admin/config/services/instagram-parser'),
      '#maxlength' => NULL,
      '#required' => TRUE,
    );

    return $form;
  }

  /**
   * Override parent::sourceFormValidate().
   */
  public function sourceFormValidate(&$values) {
  }

}
